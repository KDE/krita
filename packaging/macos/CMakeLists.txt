

set(GENERAL_ENTITLEMENTS "
    <key>com.apple.security.get-task-allow</key>
    <true/>
    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>
    <true/>
    <key>com.apple.security.cs.allow-jit</key>
    <true/>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
    <key>com.apple.security.cs.allow-dyld-environment-variables</key>
    <true/>
")

set(SANDBOX_ENTITLEMENTS "
    <!-- Sandbox entitlements -->
    <key>com.apple.security.app-sandbox</key>
    <true/>
    <key>com.apple.security.network.client</key>
    <true/>
    <key>com.apple.security.files.user-selected.read-write</key>
    <true/>
    <key>com.apple.security.files.bookmarks.app-scope</key>
    <true/>
    <key>com.apple.security.files.bookmarks.document-scope</key>
    <true/>
")

set(DEV_SANDBOX_ENTITLEMENTS "
    <!-- Temporary permissions sandbox migration -->
    <key>com.apple.security.temporary-exception.files.absolute-path.read-write</key>
    <array>
        <string>${CMAKE_INSTALL_PREFIX}/</string>
    </array>
")


# Debugging in a full sandboxed bundle is slow, instead we sign the krita
# binary using this set of entitlements, which gives permission to the entire
# BUILDROOT/i path location. This ensures krita is run in sandboxed environment
# and proper debug works.
set(ENTITLEMENTS "${GENERAL_ENTITLEMENTS} ${SANDBOX_ENTITLEMENTS} ${DEV_SANDBOX_ENTITLEMENTS}")
configure_file(entitlements.plist.template ${CMAKE_CURRENT_BINARY_DIR}/sandboxdev-entitlements.plist)

