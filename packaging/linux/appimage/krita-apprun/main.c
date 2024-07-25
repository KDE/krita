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

void try_replace_library(const char *library_name, const char *fallback_dir_name)
{
    char buf[PATH_MAX] = "";
    char real_library_path[PATH_MAX];

    void *handle = dlopen(library_name, RTLD_LAZY);

    if (handle) {
         struct link_map *info;
         int result = dlinfo(handle, RTLD_DI_LINKMAP, &info);

         if (result == 0) {
            if (realpath(info->l_name, real_library_path)) {
                // noop
            } else {
                die("Couldn't resolve the name of system %s", library_name);
            }
         } else {
            die("Couldn't get info for %s", library_name);
         }
    } else {
         die("Couldn't load %s", library_name);
    }

    char appdir_buf[PATH_MAX] = "";
    char *appdir = dirname(realpath("/proc/self/exe", appdir_buf));
    if (!appdir) {
         die("Could not access /proc/self/exe\n");
    }

    char fallback_library[PATH_MAX] = "";
    snprintf(fallback_library, PATH_MAX, "%s/usr/%s/%s", appdir, fallback_dir_name, library_name);

    if (realpath(fallback_library, buf)) {
         strcpy(fallback_library, buf);
    } else {
         die("Could not access $(APPDIR)/usr/%s/%s\n", fallback_dir_name, library_name);
    }


    const int version_offset = strlen(library_name) + 1;
    char *fallback_library_name = strstr(fallback_library, library_name);
    char *real_library_name = strstr(real_library_path, library_name);

//    printf("fallback_library_name %s\n", fallback_library_name);
//    printf("real_library_name %s\n", real_library_name);

    char *old_env;
    size_t length;
    const char *format;

    if (needs_replace_library(real_library_name + version_offset, fallback_library_name + version_offset)) {
         printf("Replacing %s with the fallback version: %s -> %s\n", library_name, real_library_name, fallback_library_name);

         old_env = getenv("LD_LIBRARY_PATH");
         if (!old_env) old_env = "";

         SET_NEW_ENV(new_ld_library_path,
                     strlen(appdir) + strlen(fallback_dir_name) + strlen(old_env),
                     "LD_LIBRARY_PATH=%s/usr/%s/:%s", appdir, fallback_dir_name, old_env);
         old_env = getenv("LD_LIBRARY_PATH") ?: "";
    }
}


int main(int argc, char **argv)
{
    char appdir_buf[PATH_MAX] = "";
    char *appdir = dirname(realpath("/proc/self/exe", appdir_buf));
    if (!appdir) {
         die("Could not access /proc/self/exe\n");
    }

    try_replace_library("libstdc++.so.6", "libstdcpp-fallback");

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
