#include <stdio.h>
/*
 * cJSON Usage Example
 * cJSON version: 1.7.19
 * Compile: gcc -o cjson_demo cjson_demo.c cJSON.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

/* ============================================================
 * Example 1: Parse JSON string
 * ============================================================ */
void example_parse(void)
{
    const char *json_str =
        "{"
        "\"name\": \"EV Charger\","
        "\"model\": \"AC-7kW\","
        "\"power\": 7.0,"
        "\"online\": true,"
        "\"ports\": 2,"
        "\"location\": null,"
        "\"features\": [\"OCPP\", \"RFID\", \"4G\"]"
        "}";

    printf("===== Example 1: Parse JSON =====\n");

    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Parse error before: %s\n", error_ptr);
        }
        return;
    }

    /* Get string value */
    cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");
    if (cJSON_IsString(name)) {
        printf("name: %s\n", name->valuestring);
    }

    /* Get number value */
    cJSON *power = cJSON_GetObjectItemCaseSensitive(root, "power");
    if (cJSON_IsNumber(power)) {
        printf("power: %.1f kW\n", power->valuedouble);
    }

    /* Get boolean value */
    cJSON *online = cJSON_GetObjectItemCaseSensitive(root, "online");
    if (cJSON_IsBool(online)) {
        printf("online: %s\n", cJSON_IsTrue(online) ? "yes" : "no");
    }

    /* Get integer value */
    cJSON *ports = cJSON_GetObjectItemCaseSensitive(root, "ports");
    if (cJSON_IsNumber(ports)) {
        printf("ports: %d\n", ports->valueint);
    }

    /* Check null */
    cJSON *location = cJSON_GetObjectItemCaseSensitive(root, "location");
    if (cJSON_IsNull(location)) {
        printf("location: null\n");
    }

    /* Parse array */
    cJSON *features = cJSON_GetObjectItemCaseSensitive(root, "features");
    if (cJSON_IsArray(features)) {
        int array_size = cJSON_GetArraySize(features);
        printf("features (%d items):\n", array_size);

        cJSON *item;
        cJSON_ArrayForEach(item, features) {
            if (cJSON_IsString(item)) {
                printf("  - %s\n", item->valuestring);
            }
        }
    }

    /* Free memory */
    cJSON_Delete(root);
    printf("\n");
}

/* ============================================================
 * Example 2: Build JSON object
 * ============================================================ */
void example_build(void)
{
    printf("===== Example 2: Build JSON =====\n");

    /* Create root object */
    cJSON *root = cJSON_CreateObject();

    /* Add string */
    cJSON_AddStringToObject(root, "name", "EV Charger");

    /* Add number */
    cJSON_AddNumberToObject(root, "power", 7.0);
    cJSON_AddNumberToObject(root, "ports", 2);

    /* Add boolean */
    cJSON_AddBoolToObject(root, "online", 1);   /* true */
    cJSON_AddBoolToObject(root, "fault", 0);    /* false */

    /* Add null */
    cJSON_AddNullToObject(root, "firmware");

    /* Add array */
    cJSON *features = cJSON_AddArrayToObject(root, "features");
    cJSON_AddItemToArray(features, cJSON_CreateString("OCPP 1.6J"));
    cJSON_AddItemToArray(features, cJSON_CreateString("RFID"));
    cJSON_AddItemToArray(features, cJSON_CreateString("4G"));
    cJSON_AddItemToArray(features, cJSON_CreateNumber(100));

    /* Add nested object */
    cJSON *status = cJSON_AddObjectToObject(root, "status");
    cJSON_AddStringToObject(status, "state", "Charging");
    cJSON_AddNumberToObject(status, "current", 32.5);
    cJSON_AddNumberToObject(status, "voltage", 220.0);

    /* Convert to string (formatted) */
    char *json_str = cJSON_Print(root);
    if (json_str != NULL) {
        printf("%s\n", json_str);
        free(json_str);  /* cJSON_Print allocates memory, must free */
    }

    /* Convert to string (compact, no whitespace) */
    char *compact_str = cJSON_PrintUnformatted(root);
    if (compact_str != NULL) {
        printf("\nCompact: %s\n", compact_str);
        free(compact_str);
    }

    /* Free memory */
    cJSON_Delete(root);
    printf("\n");
}

/* ============================================================
 * Example 3: Array operations
 * ============================================================ */
void example_array(void)
{
    printf("===== Example 3: Array Operations =====\n");

    /* Create array with values directly */
    cJSON *arr = cJSON_CreateIntArray((const int[]){1, 2, 3, 4, 5}, 5);

    /* Print array */
    char *str = cJSON_Print(arr);
    printf("Original: %s\n", str);
    free(str);

    /* Access by index */
    cJSON *item = cJSON_GetArrayItem(arr, 2);
    if (item != NULL) {
        printf("Item at index 2: %d\n", item->valueint);
    }

    /* Insert at position */
    cJSON_InsertItemInArray(arr, 2, cJSON_CreateNumber(99));
    str = cJSON_Print(arr);
    printf("After insert: %s\n", str);
    free(str);

    /* Replace item */
    cJSON_ReplaceItemInArray(arr, 0, cJSON_CreateString("first"));
    str = cJSON_Print(arr);
    printf("After replace: %s\n", str);
    free(str);

    /* Delete item */
    cJSON_DeleteItemFromArray(arr, 1);
    str = cJSON_Print(arr);
    printf("After delete: %s\n", str);
    free(str);

    cJSON_Delete(arr);
    printf("\n");
}

/* ============================================================
 * Example 4: Modify and query
 * ============================================================ */
void example_modify(void)
{
    printf("===== Example 4: Modify & Query =====\n");

    const char *json_str =
        "{\"id\": 1001, \"name\": \"Charger A\", \"status\": \"Idle\"}";

    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) return;

    /* Check if key exists */
    if (cJSON_HasObjectItem(root, "name")) {
        printf("Key 'name' exists\n");
    }

    /* Modify existing value */
    cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
    if (status != NULL) {
        cJSON_SetValuestring(status, "Charging");
    }

    /* Add new field */
    cJSON_AddNumberToObject(root, "power", 7.5);

    /* Delete field */
    cJSON_DeleteItemFromObject(root, "id");

    /* Print result */
    char *result = cJSON_Print(root);
    printf("Modified: %s\n", result);
    free(result);

    /* Get object size */
    int size = cJSON_GetArraySize(root);  /* object size also uses this */
    printf("Object has %d keys\n", size);

    /* Iterate all keys */
    cJSON *item;
    printf("All keys:\n");
    cJSON_ArrayForEach(item, root) {
        printf("  %s: ", item->string);
        if (cJSON_IsString(item)) printf("%s", item->valuestring);
        else if (cJSON_IsNumber(item)) printf("%.1f", item->valuedouble);
        printf("\n");
    }

    cJSON_Delete(root);
    printf("\n");
}

/* ============================================================
 * Example 5: Deep copy and comparison
 * ============================================================ */
void example_deepcopy(void)
{
    printf("===== Example 5: Deep Copy =====\n");

    cJSON *original = cJSON_CreateObject();
    cJSON_AddStringToObject(original, "name", "Test");
    cJSON_AddNumberToObject(original, "value", 42);

    /* Deep copy */
    cJSON *copy = cJSON_Duplicate(original, 1);  /* 1 = deep copy */

    /* Modify copy doesn't affect original */
    cJSON *val = cJSON_GetObjectItemCaseSensitive(copy, "value");
    if (val != NULL) {
        cJSON_SetNumberValue(val, 999);
    }

    char *orig_str = cJSON_Print(original);
    char *copy_str = cJSON_Print(copy);
    printf("Original: %s\n", orig_str);
    printf("Copy:     %s\n", copy_str);
    free(orig_str);
    free(copy_str);

    /* Compare two cJSON objects */
    if (cJSON_Compare(original, copy, 0)) {
        printf("Objects are equal\n");
    } else {
        printf("Objects are different\n");
    }

    cJSON_Delete(original);
    cJSON_Delete(copy);
    printf("\n");
}

/* ============================================================
 * Main
 * ============================================================ */
int main(void)
{
    printf("cJSON Version: %s\n\n", cJSON_Version());

    example_parse();
    example_build();
    example_array();
    example_modify();
    example_deepcopy();

    return 0;
}
