# SPDX-FileCopyrightText: 2029 Rebecca Breu <rebecca@rbreu.de>

# This file is part of Krita.

# Krita is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Krita is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Krita.  If not, see <https://www.gnu.org/licenses/>.

import mimetypes
import os
import urllib
import urllib.request


class PluginDownloadError(Exception):
    """Base class for all exceptions of this module."""
    pass


def is_zip(url):
    """Check if the given URL is a direct link to a zip file"""

    MTYPE = 'application/zip'

    # This just goes by the ending of the url string:
    if mimetypes.guess_type(url)[0] == MTYPE:
        return True

    # Could still be a zip, so check the HTTP headers:
    try:
        request = urllib.request.Request(url, method='HEAD')
        response = urllib.request.urlopen(request)
    except Exception as e:
        raise PluginDownloadError(str(e))

    return response.getheader('Content-Type') == MTYPE


def get_zipurl_github(base_path):
    """Guess the zip location from a github url"""

    url = None
    split = base_path.split('/')
    if len(split) > 2:
        url = (f'https://api.github.com/repos/{split[1]}/{split[2]}/'
               'zipball/master')
    return (url, {'Accept': 'application/vnd.github.v3+json'})


def get_zipurl(url):
    """Guess the zip location from a given URL."""

    if is_zip(url):
        return (url, {})

    parsed = urllib.parse.urlparse(url)
    if parsed.netloc == 'github.com':
        return get_zipurl_github(parsed.path)

    raise PluginDownloadError(
        i18n('Could not determine download link from URL'))


def download_plugin(url, dest_dir):
    """Download a plugin from a given URL into the given directory.

    ``url`` may either point directly to a zip location (on any site),
    or to a github repository.

    Returns full path of the downloaded zip file.
    """

    dest_path = os.path.join(dest_dir, 'plugin.zip')
    zip_url, headers = get_zipurl(url)
    headers['User-Agent'] = 'krita-plugin-importer'

    try:
        request = urllib.request.Request(zip_url, headers=headers)
        with urllib.request.urlopen(request) as source:
            with open(dest_path, 'wb') as destination:
                destination.write(source.read())
    except Exception as e:
        raise PluginDownloadError(str(e))
    return dest_path
