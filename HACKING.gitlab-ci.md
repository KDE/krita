Here is a list of guidelines that we follow when developing CI pipeline for Krita:

0) Indentation: use 2 spaces to indent the YAML code. It differs from the style we use 
   for C++ code in Krita. We just follow the style that Sysadmins use in their scripts.

   Options for VSCode:

   ```json
   "[yaml]": {
        "editor.formatOnSave": false,
        "editor.tabSize": 2,
        "editor.insertSpaces": true,
        "editor.detectIndentation": true,
        "editor.wrappingIndent": "indent",
        "editor.autoIndent": "full"
      },
   ```

1) If your build job is long enough to potentially overflow the GitLab's log limit (~4MiB),
   add `job-step.py 2>&1 | tee build-step.log` to every build step of this job. In the end
   of the job, upload these logs as artifacts. It prevents us from losing time trying to 
   resolve CI issues, when the logs are lost due to GitLab's log-size limitation.

   On Windows use a different line: 
   
   `job-step.py 2>&1 | Tee-Object -FilePath "build-step.log`

2) Try to write all the scripts in Python3, if it makes sense on the particular platform.
   Avoid Windows Batch (.cmd) scripts by all means, they are known to be non-portable and
   work differently on consumer Windows and Windows Server systems.

3) All string in .yml files should be unquoted by default; if a string contains any wildcard
   symbols (e.g. '*') or yaml's control character (e.g. ':' in Windows' paths) , wrap it into
   quotes.

    ```yaml
    windows-build:
      variables:
        KDECI_BUILD_TYPE: Release # simple string -> unquoted
        KDECI_EXTRA_CMAKE_ARGS: -DHIDE_SAFE_ASSERTS=OFF # simple string -> unquoted
        KDECI_CC_CACHE: "C:\\Gitlab\\Caches\\krita-windows" # Windows path -> quoted

    linux-build:
      variables:
        KDECI_CC_CACHE: /mnt/caches/krita-appimage/ # Linux path -> unquoted
    ```

4) Boolean values passed to our CI scritps should be (unquoted) `True` or `False` strings.

    ```yaml
    windows-build:
      variables:
        KDECI_COMPRESS_PACKAGES_ON_DOWNLOAD: False
        KRITACI_SKIP_UPLOAD_NIGHTLY_PACKAGE: True
    ```

5) Split CMake options passed via environment variables into multiple lines
   using `>` character:

   ```yaml
   linux-nightly:
      variables:
        KDECI_EXTRA_CMAKE_ARGS: >
          -DHIDE_SAFE_ASSERTS=OFF
          -DBUILD_TESTING=OFF
   ```