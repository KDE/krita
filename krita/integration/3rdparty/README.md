# Minizip 1.2

Zlib contribution fork that ontains the latest bug fixes that having been found all over the internet including the [old minizip forum](https://web.archive.org/web/20121015065401/http://www.winimage.info/forum/) and zlib developer's mailing list along with some additional features. Based on the original work of [Gilles Vollant](http://www.winimage.com/zLibDll/minizip.html) and contributed to by many people over the years.

## Features

### I/O Memory

To unzip from a zip file in memory use fill_memory_filefunc and supply a proper ourmemory_t structure.
```
zlib_filefunc_def filefunc32 = {0};
ourmemory_t unzmem = {0};

unzmem.size = bufsize;
unzmem.base = (char *)malloc(unzmem.size);
memcpy(unzmem.base, buffer, unzmem.size);
    
fill_memory_filefunc(&filefunc32, &unzmem);

unzOpen2("__notused__", &filefunc32);
```

To create a zip file in memory use fill_memory_filefunc and supply a proper ourmemory_t structure. It is important
not to forget to free zipmem->base when finished. If grow is set, zipmem->base will expand to fit the size of the zip. 
If grow is not set be sure to fill out zipmem.base and zipmem.size.

```
zlib_filefunc_def filefunc32 = {0};
ourmemory_t zipmem = {0};

zipmem.grow = 1;

fill_memory_filefunc(&filefunc32, &zipmem);

zipOpen3("__notused__", APPEND_STATUS_CREATE, 0, 0, &filefunc32);
```

### BZIP2

+ Requires #define HAVE_BZIP2
+ Requires BZIP2 library

### Windows RT

+ Requires #define IOWIN32_USING_WINRT_API

## Additional Features

### [WinZIP AES Encryption](http://www.winzip.com/aes_info.htm)

+ Requires #define HAVE_AES
+ Requires AES library files

When zipping with a password it will always use AES 256-bit encryption. 
When unzipping it will use AES decryption only if necessary. Does not support central directory or local file header encryption.

### PKWARE disk splitting

To create an archive with multiple disks use zipOpen3_64 supplying a disk size value in bytes.

```
extern zipFile ZEXPORT zipOpen3_64(const void *pathname, int append, 
  ZPOS64_T disk_size, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def);
```
The central directory is the only data stored in the .zip and doesn't follow disk size restrictions.

When unzipping it will automatically determine when in needs to cross disk boundaries.

### I/O Buffering

Improves I/O performance by buffering read and write operations. 
```
zlib_filefunc64_def filefunc64 = {0};
ourbuffer_t buffered = {0};
    
fill_win32_filefunc64(&buffered->filefunc64);
fill_buffer_filefunc64(&filefunc64, buffered);
    
unzOpen2_64(filename, &filefunc64)
```

### Apple libcompression

+ Requires #define HAVE_APPLE_COMPRESSION
