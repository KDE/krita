/*
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

extern "C" {
    __declspec(dllimport) int krita_main(int argc, char **argv);
}

int main(int argc, char **argv)
{
    return krita_main(argc, argv);
}
