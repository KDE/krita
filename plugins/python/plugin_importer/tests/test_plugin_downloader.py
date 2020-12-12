import httpretty
import pytest
from unittest import TestCase

from .. plugin_downloader import get_zipurl, PluginDownloadError


class GetZipurlTestCase(TestCase):

    @httpretty.activate
    def test_basic_github_url(self):
        url = 'https://github.com/foo/bar'
        httpretty.register_uri(httpretty.HEAD, url, status=200)
        result, headers = get_zipurl(url)
        assert result == 'https://api.github.com/repos/foo/bar/zipball/master'
        assert headers == {'Accept': 'application/vnd.github.v3+json'}

    @httpretty.activate
    def test_github_url_with_suburl(self):
        url = 'https://github.com/foo/bar/branches'
        httpretty.register_uri(httpretty.HEAD, url, status=200)
        result, headers = get_zipurl(url)
        assert result == 'https://api.github.com/repos/foo/bar/zipball/master'
        assert headers == {'Accept': 'application/vnd.github.v3+json'}

    @httpretty.activate
    def test_github_specific_zipfile_version(self):
        url = 'https://github.com/foo/bar/archive/1.2.3.zip'
        httpretty.register_uri(httpretty.HEAD, url, status=200)
        assert get_zipurl(url) == (url, {})

    @httpretty.activate
    def test_github_missing_repository(self):
        url = 'https://github.com/foo'
        httpretty.register_uri(httpretty.HEAD, url, status=200)
        result, headers = get_zipurl(url)
        assert result is None

    @httpretty.activate
    def test_not_github_url(self):
        url = 'https://notgithub.com/foo/bar'
        httpretty.register_uri(httpretty.HEAD, url, status=200)
        with pytest.raises(PluginDownloadError):
            get_zipurl(url)

    @httpretty.activate
    def test_not_github_specific_zipfile(self):
        url = 'https://alreadyzip.com/foo.zip'
        httpretty.register_uri(httpretty.HEAD, url, status=200)
        assert get_zipurl(url) == (url, {})

    @httpretty.activate
    def test_not_github_specific_zipfile_by_httpheader(self):
        url = 'https://alreadyzip.com/foo'
        httpretty.register_uri(httpretty.HEAD, url, status=200,
                               **{'Content-Type': 'application/zip'})
        assert get_zipurl(url) == (url, {})

    @httpretty.activate
    def test_raises_error_on_failed_status_codes(self):
        url = 'https://error.com/foo/bar'
        httpretty.register_uri(httpretty.HEAD, url, status=404)
        with pytest.raises(PluginDownloadError):
            get_zipurl(url)

    @httpretty.activate
    def test_raises_errors_on_unparsable_url(self):
        with pytest.raises(PluginDownloadError):
            get_zipurl('iae')
