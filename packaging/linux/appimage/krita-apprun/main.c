/**
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 * SPDX-FileCopyrightText: 2004-2018 Simon Peter
 * SPDX-FileCopyrightText: 2010 RazZziel
 *
 * SPDX-License-Identifier: MIT
 */

#define _GNU_SOURCE

#include <limits.h>
#include <link.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>

void split_version(const char *version, int values[4])
{
    char *ver = strdup(version);
    
    // printf("     ver: %s\n", ver);
                        
    int index = 0;
    char *token = strtok(ver, ".");
            
    while (token && index < 4) {
        const int value = atoi(token);
        
        // printf("value: %s -> %d\n", token, value);
        
        values[index] = value;
        index++;
        token = strtok(0, ".");
    }
    
    while (index < 4) {
        values[index] = 0;
        index++;
    }
        
    free (ver);
}

int needs_replace_library(const char *library, const char *base_library)
{
    int base_value[4];
    int value[4];
            
    split_version(base_library, base_value);
    split_version(library, value);
                        
    int result = 0;
            
    for (int i = 0; i < 4; i++) {
        if (base_value[i] < value[i]) {
            break;
        } else if (base_value[i] > value[i]) {
            result = 1;
            break;
         }
    }
        
    return result;
}

#define die(...)                                    \
    do {                                            \
        fprintf(stderr, "Error: " __VA_ARGS__);     \
        exit(1);                                    \
    } while(0);

#define SET_NEW_ENV(str,len,fmt,...)                \
    format = fmt;                                   \
    length = strlen(format) + (len);                \
    char *str = calloc(length, sizeof(char));     \
    snprintf(str, length, format, __VA_ARGS__);   \
    putenv(str);

int main(int argc, char **argv)
{
    char buf[PATH_MAX] = "";
    char real_library_path[PATH_MAX];

    void *handle = dlopen("libstdc++.so.6", RTLD_LAZY);

    if (handle) {
        struct link_map *info;
        int result = dlinfo(handle, RTLD_DI_LINKMAP, &info);
        
        if (result == 0) {
            if (realpath(info->l_name, real_library_path)) {
                // noop
            } else {
                die("Couldn't resolve the name of system libstdc++.so.6");
            }
        } else {
            die("Couldn't get info for libstdc++.so.6");
        }
    } else {
        die("Couldn't load libstdc++.so.6");
    }
    
    char appdir_buf[PATH_MAX] = "";
    char *appdir = dirname(realpath("/proc/self/exe", appdir_buf));
    if (!appdir) {
        die("Could not access /proc/self/exe\n");
    }

    char fallback_library[PATH_MAX] = "";
    strcat(fallback_library, appdir);
    strcat(fallback_library, "/usr/libstdcpp-fallback/libstdc++.so.6");


    if (realpath(fallback_library, buf)) {
        strcpy(fallback_library, buf);
    } else {
        die("Could not access $(APPDIR)/usr/libstdcpp-fallback/libstdc++.so.6\n");
    }

    char library_prefix[] = "libstdc++.so.";
    char *fallback_library_name = strstr(fallback_library, "libstdc++.so.6");
    char *real_library_name = strstr(real_library_path, "libstdc++.so.6");

    // printf("fallback_library_name %s\n", fallback_library_name);
    // printf("real_library_name %s\n", real_library_name);

    size_t appdir_s = strlen(appdir);

    char *old_env;
    size_t length;
    const char *format;

    if (needs_replace_library(real_library_name + sizeof(library_prefix), fallback_library_name + sizeof(library_prefix))) {
        printf("Replacing libstdc++.so.6 with the fallback version: %s -> %s\n", real_library_name, fallback_library_name);

        old_env = getenv("LD_LIBRARY_PATH") ?: "";
        SET_NEW_ENV(new_ld_library_path, appdir_s*10 + strlen(old_env), "LD_LIBRARY_PATH=%s/usr/libstdcpp-fallback/:%s", appdir, old_env);
    }

    char exec_path[PATH_MAX] = "";
    strcpy(exec_path, appdir);
    strcat(exec_path, "/usr/bin/krita");

    char *exec_args[argc + 1];
    exec_args[0] = exec_path;

    for (int i = 1; i < argc; i++) {
       exec_args[i] = argv[i];
    }

    exec_args[argc] = 0;

    int ret = execvp(exec_path, exec_args);

    if (ret == -1) {
        int error = errno;
        die("Error executing '%s': %s\n", exec_path, strerror(error));
    }

    return 0;
}
