TARGET = qtposition_geoclue2
QT = core positioning dbus

PLUGIN_TYPE = position
PLUGIN_CLASS_NAME = GeoClue2PositionPlugin
load(qt_plugin)

HEADERS += \
    qgeopositioninfosource_geoclue2_p.h \
    qgeopositioninfosourcefactory_geoclue2.h \
    geoclue2client_p.h \
    geoclue2manager_p.h \
    geoclue2location_p.h

SOURCES += \
    qgeopositioninfosource_geoclue2.cpp \
    qgeopositioninfosourcefactory_geoclue2.cpp \
    geoclue2client.cpp \
    geoclue2manager.cpp \
    geoclue2location.cpp

INCLUDEPATH += $$QT.location.includes

OTHER_FILES += \
    plugin.json
