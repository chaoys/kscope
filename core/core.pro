include(../config)
TEMPLATE = lib
TARGET = kscope_core
CONFIG += dll

# Input
HEADERS += locationtreemodel.h \
    locationmodel.h \
    projectconfig.h \
    filescanner.h \
    filefilter.h \
    queryview.h \
    locationlistmodel.h \
    parser.h \
    exception.h \
    project.h \
    codebase.h \
    codebasemodel.h \
    globals.h \
    process.h \
    statemachine.h \
    treeitem.h \
    progressbar.h \
    engine.h \
    locationview.h \
    textfilterdialog.h
FORMS += progressbar.ui \
    textfilterdialog.ui
SOURCES += locationtreemodel.cpp \
    locationmodel.cpp \
    filescanner.cpp \
    queryview.cpp \
    locationlistmodel.cpp \
    codebasemodel.cpp \
    process.cpp \
    progressbar.cpp \
    locationview.cpp \
    textfilterdialog.cpp
RESOURCES = core.qrc
target.path = $${INSTALL_PATH}/lib
INSTALLS += target
