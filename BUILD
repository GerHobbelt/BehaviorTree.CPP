licenses(["notice"]) 
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")
filegroup(
    name = "all",
    srcs =  glob(["**"])
)

BT_SOURCE = [
    "src/action_node.cpp",
    "src/basic_types.cpp",
    "src/behavior_tree.cpp",
    "src/blackboard.cpp",
    "src/bt_factory.cpp",
    "src/decorator_node.cpp",
    "src/condition_node.cpp",
    "src/control_node.cpp",
    "src/shared_library.cpp",
    "src/shared_library_UNIX.cpp",
    "src/tree_node.cpp",
    "src/xml_parsing.cpp",
    "src/decorators/inverter_node.cpp",
    "src/decorators/repeat_node.cpp",
    "src/decorators/retry_node.cpp",
    "src/decorators/subtree_node.cpp",
    "src/decorators/timeout_node.cpp",
    "src/controls/if_then_else_node.cpp",
    "src/controls/fallback_node.cpp",
    "src/controls/parallel_node.cpp",
    "src/controls/reactive_sequence.cpp",
    "src/controls/reactive_fallback.cpp",
    "src/controls/sequence_node.cpp",
    "src/controls/sequence_star_node.cpp",
    "src/loggers/bt_zmq_publisher.cpp",
    "src/controls/switch_node.cpp",
    "src/controls/while_do_else_node.cpp",
    "src/loggers/bt_cout_logger.cpp",
    "src/loggers/bt_file_logger.cpp",
    "src/loggers/bt_minitrace_logger.cpp",
    "src/private/tinyxml2.cpp",
    "3rdparty/minitrace/minitrace.cpp",
]

cc_library(
    name = "behaviortree_cpp",
    srcs = BT_SOURCE,
    copts = [
        "-DBT_BOOST_COROUTINE2",
        "-DZMQ_FOUND",
    ],
    linkopts = [
        "-fcoroutines",
    ],
    hdrs = glob(["include/behaviortree_cpp_v3/**", "3rdparty/**/*.h","src/private/*.h"]),
    includes = [
        "include",
        "3rdparty",
        "src",
    ],
    deps = [
        "@boost//:coroutine2",
        "@cppzmq",
    ],
    visibility = [
        "//visibility:public",
    ]
)

