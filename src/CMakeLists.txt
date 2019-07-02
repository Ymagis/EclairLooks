
find_package(Qt5Widgets)
find_package(Qt5UiTools)

configure_file("version.h.in" "${CMAKE_CURRENT_LIST_DIR}/version.h" @ONLY)

set(SOURCES
    main.cpp
    context.cpp

    # Core #
    core/image.cpp
    core/imagepipeline.cpp

    # Gui #
    gui/common/common.cpp
    gui/common/browser.cpp
    gui/common/imageviewer.cpp
    gui/common/setting.cpp
    gui/common/slider.cpp
    gui/common/textureview.cpp

    gui/scope/cube.cpp
    gui/scope/neutral.cpp
    gui/scope/waveform.cpp
    gui/scope/vectorscope.cpp

    gui/view/dev/operator.cpp
    gui/view/dev/operatorlist.cpp
    gui/view/dev/pipeline.cpp
    gui/view/dev/widget.cpp

    gui/view/log/widget.cpp

    gui/view/look/detail.cpp
    gui/view/look/listview.cpp
    gui/view/look/selection.cpp
    gui/view/look/tabview.cpp
    gui/view/look/widget.cpp

    gui/mainwindow.cpp
    gui/uiloader.cpp

    # Operator #
    operator/ctl/operator.cpp
    operator/ctl/transform.cpp

    operator/ocio/filetransform.cpp
    operator/ocio/colorspace.cpp
    operator/ocio/matrix.cpp

    operator/imageoperator.cpp
    operator/imageoperatorlist.cpp

    # Parameter #
    parameter/parameter.cpp
    parameter/parameterlist.cpp
    parameter/parameterseriallist.cpp
    parameter/parameterwidget.cpp
    parameter/checkbox/parameter.cpp
    parameter/checkbox/widget.cpp
    parameter/filepath/parameter.cpp
    parameter/filepath/widget.cpp
    parameter/matrix/parameter.cpp
    parameter/matrix/widget.cpp
    parameter/select/parameter.cpp
    parameter/select/widget.cpp
    parameter/slider/parameter.cpp
    parameter/slider/widget.cpp
    parameter/text/parameter.cpp
    parameter/text/widget.cpp

    # Utils #
    utils/chrono.cpp
    utils/pystring.cpp
)

qt5_add_resources(RESOURCES resources/res.qrc)

set(MACOSX_BUNDLE_BUNDLE_NAME "Eclair Looks")
set(MACOSX_BUNDLE_ICON_FILE hexa.icns)
set(ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/hexa.icns)
set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${RESOURCES} ${ICON})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        opencolorio
        openimageio
        openexr
        ctl
        Qt5::Widgets
        Qt5::UiTools
        ${CXX_FILESYSTEM_LIB}
)