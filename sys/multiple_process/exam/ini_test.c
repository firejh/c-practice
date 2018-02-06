#include "ini.h"

const char *ini_config = "hover     =      lees       \n"  \
        " data =    7 \n"   \
        " maxthread=255\n"  \
        ";this is a comment line\r\n"   \
        ";中文注释\n"   \
        "yahoo    =alibaba     \n"  \
        "     [section1]   \n"  \
        "hover   =  lees2  \n"  \
        "hover=lees333\n\n" \
        " hover = lees4444\n"   \
        "yahoo=3\n\n\n";

const char *conf_file_name = "/tmp/test.ini";

int main(int argc,char* argv[])
{
    INI_CONFIG *config;
    printf("------------test1-----------\n");
    config = ini_config_create_from_string((unsigned char*)(ini_config), 0, 0);
    if (config) {
        ini_config_print(config, stdout);
        ini_config_save(config, conf_file_name);
        ini_config_destroy(config);
    }

    printf("\n------------test2-----------\n");
    config = ini_config_create_from_file(conf_file_name, 0);
    if (config) {
        printf(
            "%s %s\n",
            ini_config_get_string(config, NULL, "maxthread", "5"),
            ini_config_get_string(config, "section1", "hover", "lee")
        );
        ini_config_destroy(config);
    }

    return 0;
}
