/****************************************************************************
** Meta object code from reading C++ file 'camera.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.10)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "camera.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'camera.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.10. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_camera_t {
    QByteArrayData data[12];
    char stringdata0[107];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_camera_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_camera_t qt_meta_stringdata_camera = {
    {
QT_MOC_LITERAL(0, 0, 6), // "camera"
QT_MOC_LITERAL(1, 7, 8), // "newFrame"
QT_MOC_LITERAL(2, 16, 0), // ""
QT_MOC_LITERAL(3, 17, 10), // "fpsChanged"
QT_MOC_LITERAL(4, 28, 3), // "fps"
QT_MOC_LITERAL(5, 32, 13), // "frameCaptured"
QT_MOC_LITERAL(6, 46, 8), // "cv::Mat*"
QT_MOC_LITERAL(7, 55, 4), // "data"
QT_MOC_LITERAL(8, 60, 11), // "updateFrame"
QT_MOC_LITERAL(9, 72, 12), // "calculateFPS"
QT_MOC_LITERAL(10, 85, 17), // "cv::VideoCapture&"
QT_MOC_LITERAL(11, 103, 3) // "cap"

    },
    "camera\0newFrame\0\0fpsChanged\0fps\0"
    "frameCaptured\0cv::Mat*\0data\0updateFrame\0"
    "calculateFPS\0cv::VideoCapture&\0cap"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_camera[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,
       3,    1,   40,    2, 0x06 /* Public */,
       5,    1,   43,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   46,    2, 0x0a /* Public */,
       9,    1,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,    4,
    QMetaType::Void, 0x80000000 | 6,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,

       0        // eod
};

void camera::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<camera *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->newFrame(); break;
        case 1: _t->fpsChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->frameCaptured((*reinterpret_cast< cv::Mat*(*)>(_a[1]))); break;
        case 3: _t->updateFrame(); break;
        case 4: _t->calculateFPS((*reinterpret_cast< cv::VideoCapture(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (camera::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&camera::newFrame)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (camera::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&camera::fpsChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (camera::*)(cv::Mat * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&camera::frameCaptured)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject camera::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_camera.data,
    qt_meta_data_camera,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *camera::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *camera::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_camera.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int camera::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void camera::newFrame()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void camera::fpsChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void camera::frameCaptured(cv::Mat * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
