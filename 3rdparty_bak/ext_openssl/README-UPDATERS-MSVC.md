Notes on updating OpenSSL
---

The binaries that we use for MSVC were copied from the Conan package manager, the package listing of which is at:

https://conan.io/center/openssl?os=Windows&tab=configuration

For reasons of transparency and reproducibility, I attach the full configuration I used below.

Configuration:

    Windows / Any / Visual Studio

The entry in "List of available binary packages":

    openssl/1.1.1k:6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7 

`conanfile.txt`:

    [requires]
    openssl/1.1.1k

    [options]
    openssl:shared=True

    [generators]
    cmake

The configuration above should result in the following lock files:

`conaninfo.txt`:

    [settings]
        arch=x86_64
        build_type=Release
        compiler=Visual Studio
        compiler.runtime=MD
        compiler.version=15
        os=Windows

    [requires]
        openssl/1.Y.Z

    [options]


    [full_settings]
        arch=x86_64
        arch_build=x86_64
        build_type=Release
        compiler=Visual Studio
        compiler.runtime=MD
        compiler.version=15
        os=Windows
        os_build=Windows

    [full_requires]
        openssl/1.1.1k:970e773c5651dc2560f86200a4ea56c23f568ff9

    [full_options]
        openssl:386=False
        openssl:capieng_dialog=False
        openssl:enable_capieng=False
        openssl:no_aria=False
        openssl:no_asm=False
        openssl:no_async=False
        openssl:no_bf=False
        openssl:no_blake2=False
        openssl:no_camellia=False
        openssl:no_cast=False
        openssl:no_chacha=False
        openssl:no_cms=False
        openssl:no_comp=False
        openssl:no_ct=False
        openssl:no_deprecated=False
        openssl:no_des=False
        openssl:no_dgram=False
        openssl:no_dh=False
        openssl:no_dsa=False
        openssl:no_dso=False
        openssl:no_engine=False
        openssl:no_filenames=False
        openssl:no_gost=False
        openssl:no_hmac=False
        openssl:no_idea=False
        openssl:no_md4=False
        openssl:no_md5=False
        openssl:no_mdc2=False
        openssl:no_ocsp=False
        openssl:no_pinshared=False
        openssl:no_rc2=False
        openssl:no_rmd160=False
        openssl:no_rsa=False
        openssl:no_sha=False
        openssl:no_sm2=False
        openssl:no_sm3=False
        openssl:no_sm4=False
        openssl:no_srp=False
        openssl:no_srtp=False
        openssl:no_sse2=False
        openssl:no_ssl=False
        openssl:no_threads=False
        openssl:no_ts=False
        openssl:no_whirlpool=False
        openssl:openssldir=None
        openssl:shared=True

    [recipe_hash]


    [env]


The associated configuration ("Conan Info") for the hash above is:

    [settings]
        arch=x86_64
        build_type=Release
        compiler=Visual Studio
        compiler.runtime=MD
        compiler.version=15
        os=Windows

    [requires]


    [options]
        386=False
        capieng_dialog=False
        enable_capieng=False
        no_aria=False
        no_asm=False
        no_async=False
        no_bf=False
        no_blake2=False
        no_camellia=False
        no_cast=False
        no_chacha=False
        no_cms=False
        no_comp=False
        no_ct=False
        no_deprecated=False
        no_des=False
        no_dgram=False
        no_dh=False
        no_dsa=False
        no_dso=False
        no_engine=False
        no_filenames=False
        no_gost=False
        no_hmac=False
        no_idea=False
        no_md4=False
        no_md5=False
        no_mdc2=False
        no_ocsp=False
        no_pinshared=False
        no_rc2=False
        no_rmd160=False
        no_rsa=False
        no_sha=False
        no_sm2=False
        no_sm3=False
        no_sm4=False
        no_srp=False
        no_srtp=False
        no_sse2=False
        no_ssl=False
        no_threads=False
        no_ts=False
        no_whirlpool=False
        openssldir=None
        shared=False

    [full_settings]
        arch=x86_64
        build_type=Release
        compiler=Visual Studio
        compiler.runtime=MD
        compiler.version=15
        os=Windows

    [full_requires]


    [full_options]
        386=False
        capieng_dialog=False
        enable_capieng=False
        no_aria=False
        no_asm=False
        no_async=False
        no_bf=False
        no_blake2=False
        no_camellia=False
        no_cast=False
        no_chacha=False
        no_cms=False
        no_comp=False
        no_ct=False
        no_deprecated=False
        no_des=False
        no_dgram=False
        no_dh=False
        no_dsa=False
        no_dso=False
        no_engine=False
        no_filenames=False
        no_gost=False
        no_hmac=False
        no_idea=False
        no_md4=False
        no_md5=False
        no_mdc2=False
        no_ocsp=False
        no_pinshared=False
        no_rc2=False
        no_rmd160=False
        no_rsa=False
        no_sha=False
        no_sm2=False
        no_sm3=False
        no_sm4=False
        no_srp=False
        no_srtp=False
        no_sse2=False
        no_ssl=False
        no_threads=False
        no_ts=False
        no_whirlpool=False
        openssldir=None
        shared=False

    [recipe_hash]
        688f7f1454ac56b46b0276cea3300296

    [env]

--amyspark, Apr 8, 2021
