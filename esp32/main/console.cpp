#include "console.h"
#include "defs.h"

#include <string>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"

#include <esp_vfs_dev.h>
#include <driver/uart.h>
#include <driver/uart_vfs.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

struct
{
    struct arg_str* ssid;
    struct arg_str* password;
    struct arg_end* end;
} add_wifi_credentials_args;

int add_wifi_credentials(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &add_wifi_credentials_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, add_wifi_credentials_args.end, argv[0]);
        return 1;
    }
    const auto ssid = add_wifi_credentials_args.ssid->sval[0];
    const auto password = add_wifi_credentials_args.password->sval[0];
    if (strlen(ssid) < 1)
    {
        printf("ERROR: Invalid SSID value\n");
        return 1;
    }
    add_wifi_credentials(ssid, password);
    printf("OK: Added WiFi credentials %s/%s\n", ssid, password);
    return 0;
}

int list_wifi_creds(int argc, char** argv)
{
    const auto creds = get_wifi_creds();
    for (const auto& c : creds)
    {
        printf("%-20s %s\n", c.first.c_str(),
               c.second.empty() ? "" : "***");
    }
    printf("OK: Listed %d WiFi credentials\n", static_cast<int>(creds.size()));
    return 0;
}

int clear_wifi_credentials(int, char**)
{
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, WIFI_KEY, ""));
    nvs_close(my_handle);
    printf("OK: WiFi credentials cleared\n");
    return 0;
}

struct
{
    struct arg_str* access_key;
    struct arg_str* secret_key;
    struct arg_end* end;
} set_s3_credentials_args;

int set_s3_credentials(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &set_s3_credentials_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, set_s3_credentials_args.end, argv[0]);
        return 1;
    }
    const auto access_key = set_s3_credentials_args.access_key->sval[0];
    const auto secret_key = set_s3_credentials_args.secret_key->sval[0];
    if (strlen(access_key) < 1)
    {
        printf("ERROR: Invalid access key\n");
        return 1;
    }
    if (strlen(secret_key) < 20)
    {
        printf("ERROR: Invalid secret key\n");
        return 1;
    }
    set_s3_access_key(access_key);
    set_s3_secret_key(secret_key);
    printf("OK: S3 credentials set to %s/%s\n", access_key, secret_key);
    return 0;
}

struct
{
    struct arg_int* instance;
    struct arg_end* end;
} set_instance_args;

static int set_instance(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &set_instance_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, set_instance_args.end, argv[0]);
        return 1;
    }
    const auto inst_no = set_instance_args.instance->ival[0];
    if (inst_no < 0 || inst_no > 255)
    {
        printf("ERROR: Invalid instance number\n");
        return 1;
    }
    set_instance(inst_no);
    printf("OK: Instance number set to %d\n", inst_no);
    return 0;
}

struct
{
    struct arg_str* address;
    struct arg_end* end;
} set_mqtt_params_args;

int set_mqtt_params(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &set_mqtt_params_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, set_mqtt_params_args.end, argv[0]);
        return 1;
    }
    const auto address = set_mqtt_params_args.address->sval[0];
    set_mqtt_address(address);
    printf("OK: MQTT address set to %s\n", address);
    return 0;
}

int reboot(int, char**)
{
    printf("Reboot...\n");
    esp_restart();
    return 0;
}

void initialize_console()
{
    setvbuf(stdin, NULL, _IONBF, 0);

    uart_vfs_dev_port_set_rx_line_endings(0, ESP_LINE_ENDINGS_CR);
    uart_vfs_dev_port_set_tx_line_endings(0, ESP_LINE_ENDINGS_CRLF);

    uart_config_t uart_config;
    memset(&uart_config, 0, sizeof(uart_config));
    uart_config.baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.source_clk = UART_SCLK_REF_TICK;
    ESP_ERROR_CHECK(uart_param_config((uart_port_t) CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_driver_install((uart_port_t) CONFIG_ESP_CONSOLE_UART_NUM,
                                         256, 0, 0, NULL, 0));

    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    esp_console_config_t console_config;
    memset(&console_config, 0, sizeof(console_config));
    console_config.max_cmdline_args = 8;
    console_config.max_cmdline_length = 256;
#if CONFIG_LOG_COLORS
    console_config.hint_color = atoi(LOG_COLOR_CYAN);
#endif
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    linenoiseSetMultiLine(1);
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
    linenoiseHistorySetMaxLen(100);
}

void run_console()
{
    initialize_console();

    esp_console_register_help_command();

    add_wifi_credentials_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID");
    add_wifi_credentials_args.password = arg_strn(NULL, NULL, "<password>", 0, 1, "Password");
    add_wifi_credentials_args.end = arg_end(2);
    const esp_console_cmd_t add_wifi_credentials_cmd = {
        .command = "wifi",
        .help = "Add WiFi credentials",
        .hint = nullptr,
        .func = &add_wifi_credentials,
        .argtable = &add_wifi_credentials_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&add_wifi_credentials_cmd));

    const esp_console_cmd_t list_wifi_credentials_cmd = {
        .command = "list_wifi",
        .help = "List WiFi credentials",
        .hint = nullptr,
        .func = &list_wifi_creds,
        .argtable = nullptr,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&list_wifi_credentials_cmd));

    const esp_console_cmd_t clear_wifi_credentials_cmd = {
        .command = "clearwifi",
        .help = "Clear WiFi credentials",
        .hint = nullptr,
        .func = &clear_wifi_credentials,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&clear_wifi_credentials_cmd));

    set_s3_credentials_args.access_key = arg_str1(NULL, NULL, "<access key>", "S3 access key");
    set_s3_credentials_args.secret_key = arg_str1(NULL, NULL, "<secret key>", "S3 secret key");
    set_s3_credentials_args.end = arg_end(2);
    const esp_console_cmd_t set_s3_credentials_cmd = {
        .command = "s3",
        .help = "Set S3 credentials",
        .hint = nullptr,
        .func = &set_s3_credentials,
        .argtable = &set_s3_credentials_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_s3_credentials_cmd));

    set_instance_args.instance = arg_int1(NULL, NULL, "<instance>", "Instance number (0-255)");
    set_instance_args.end = arg_end(2);
    const esp_console_cmd_t set_instance_cmd = {
        .command = "set_instance",
        .help = "Set instance number",
        .hint = nullptr,
        .func = &set_instance,
        .argtable = &set_instance_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_instance_cmd));

    set_mqtt_params_args.address = arg_str1(NULL, NULL, "<address>", "MQTT address");
    set_mqtt_params_args.end = arg_end(2);
    const esp_console_cmd_t set_mqtt_params_cmd = {
        .command = "mqtt",
        .help = "Set MQTT host",
        .hint = nullptr,
        .func = &set_mqtt_params,
        .argtable = &set_mqtt_params_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_mqtt_params_cmd));

    const esp_console_cmd_t reboot_cmd = {
        .command = "reboot",
        .help = "Reboot",
        .hint = nullptr,
        .func = &reboot,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&reboot_cmd));

    const char* prompt = LOG_COLOR_I "hal32cam> " LOG_RESET_COLOR;
    int probe_status = linenoiseProbe();
    if (probe_status)
    {
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = "esp32> ";
#endif //CONFIG_LOG_COLORS
    }

    while (true)
    {
        char* line = linenoise(prompt);
        if (!line)
            continue;

        linenoiseHistoryAdd(line);

        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND)
            printf("Unrecognized command\n");
        else if (err == ESP_ERR_INVALID_ARG)
            ; // command was empty
        else if (err == ESP_OK && ret != ESP_OK)
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(err));
        else if (err != ESP_OK)
            printf("Internal error: %s\n", esp_err_to_name(err));

        linenoiseFree(line);
    }
}

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
