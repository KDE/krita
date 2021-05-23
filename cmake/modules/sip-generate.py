#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2019 Riverbank Computing Limited
# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: GPL-2.0-or-later

from sipbuild.abstract_project import AbstractProject
from sipbuild.exceptions import handle_exception

def main():
    """ Generate the project bindings from the command line. """

    try:
        project = AbstractProject.bootstrap(
            'build', "Generate the project bindings.")
        project.builder._generate_bindings()
        project.progress("The project bindings are ready for build.")
    except Exception as e:
        handle_exception(e)

    return 0

if __name__ == "__main__":
    main()
