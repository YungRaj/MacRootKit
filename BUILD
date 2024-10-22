load("@build_bazel_rules_apple//apple:macos.bzl", "macos_kernel_extension")

genrule(
    name = "capstone_x86_64",
    srcs = ["capstone"],
    outs = ["libcapstone_x86_64.a"],
    cmd = """
        cd capstone
        export ARCH=x86_64
        export CFLAGS="-target $$ARCH-apple-macos"
        export CXXFLAGS="-target $$ARCH-apple-macos"
        export LDFLAGS="-target $$ARCH-apple-macos"
        make clean
        CAPSTONE_ARCHS=x86 ./make.sh osx-kernel
        cd ..
        cp capstone/libcapstone.a $(OUTS)
    """,
    tags = ["no-sandbox"],
)

genrule(
    name = "capstone_arm64",
    srcs = ["capstone"],
    outs = ["libcapstone_arm64.a"],
    cmd = """
        cd capstone
        export ARCH=arm64
        export CFLAGS="-target $$ARCH-apple-macos"
        export CXXFLAGS="-target $$ARCH-apple-macos"
        export LDFLAGS="-target $$ARCH-apple-macos"
        make clean
        CAPSTONE_ARCHS=aarch64 ./make.sh osx-kernel
        cd ..
        cp capstone/libcapstone.a $(OUTS)
    """,
    tags = ["no-sandbox"],
)

genrule(
    name = "capstone_fat",
    srcs = [
        ":capstone_arm64",
        ":capstone_x86_64",
    ],
    outs = ["libcapstone.a"],
    cmd = """
        lipo -create -output $(OUTS) $(SRCS)
    """,
)

cc_library(
    name = "capstone_fat_static",
    srcs = [],
    hdrs = [],
    linkstatic = True,
    alwayslink = True,
    deps = [":capstone_fat"],
)

cc_library(
    name = "MacRootKit_library",
    srcs = glob(["kernel/*.c"]),
    hdrs = glob(["kernel/*.h"]) + glob(["mac_rootkit/*.h"]),
    includes = [
        "kernel",
        "mac_rootkit",
        "/usr/include",
        "/usr/local/include",
        "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include",
    ],
    copts = [
        "-w",
        "-mkernel", "-D__KERNEL__",
        "-nostdlib",
        "-I./",
        "-I./capstone/include",
        "-isystem", "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/Kernel.framework/Headers",
        "-isystem", "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/IOKit.framework/Headers",
        "-DCAPSTONE_HAS_X86",
        "-DCAPSTONE_HAS_ARM64",
        "-DCAPSTONE_HAS_OSXKERNEL=1",
    ],
    linkopts = [
        "-framework", "IOKit",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "MacRootKit_kext",
    deps = [
        ":MacRootKit_library",
    ],
    srcs = glob(["kernel/*.cpp"]) + glob(["mac_rootkit/*.cpp"]) + glob(["arm64/*.cpp"]) + glob(["arm64/*.s"]) + glob(["x86_64/*.cpp"]) + glob(["x86_64/*.c"]),
    hdrs = glob(["kernel/*.hpp"]) + glob(["mac_rootkit/*.hpp"]) + glob(["kernel/*.h"]) + glob(["mac_rootkit/*.h"]) + glob(["arm64/*.hpp"]) + glob(["x86_64/*.hpp"]) + glob(["capstone/include/capstone/*.h"]),
    includes = [
        "kernel",
        "mac_rootkit",
        "/usr/include",
        "/usr/local/include",
        "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include",
    ],
    copts = [
        "-w",
        "-Xlinker",
        "-kext",
        "-Xlinker",
        "-export_dynamic",
        "-Wl,-kext",
        "-lkmod",
        "-lkmodc++", 
        "-lcc_kext",
        "-std=c++20",
        "-mkernel", "-D__KERNEL__",
        "-nostdlib",
        "-I./",
        "-I./capstone/include",
        "-isystem", "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/Kernel.framework/Headers",
        "-isystem", "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/IOKit.framework/Headers",
        "-DCAPSTONE_HAS_X86",
        "-DCAPSTONE_HAS_ARM64",
        "-DCAPSTONE_HAS_OSXKERNEL=1",
    ],
    linkopts = [
        "-framework", "IOKit",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

macos_kernel_extension(
    name = "MacRootKit",
    deps = [
        ":MacRootKit_kext",
        ":capstone_fat_static",
    ],
    resources = [],
    additional_contents = {},
    additional_linker_inputs = [
    ],
    bundle_id = "com.YungRaj.MacRootKit",
    bundle_id_suffix = "_",
    bundle_name = "MacRootKit",
    codesign_inputs = [],
    entitlements = "entitlements.xml",
    entitlements_validation = "loose",
    executable_name = "MacRootKit",
    exported_symbols_lists = [],
    families = ["mac"],
    infoplists = ["Info.plist"],
    ipa_post_processor = None,
    linkopts = [
        "-framework", "IOKit",
    ],
    minimum_deployment_os_version = "",
    minimum_os_version = "11.0",
    platform_type = "macos",
    provisioning_profile = None,
    shared_capabilities = [],
    stamp = 1,
    strings = [],
    version = None,
)