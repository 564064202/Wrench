#include "adbclient.h"
#include <stdio.h>
#include <QTcpSocket>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QProcessEnvironment>
#include <QString>
#include <QRegExp>

AdbClient::AdbClient()
{
    static QString adb_serial = QProcessEnvironment::systemEnvironment().value("ANDROID_SERIAL");
    static QByteArray adb_serial_buffer = adb_serial.toUtf8();
    static bool adb_serial_inited = false;
    if (not adb_serial_inited) {
        adb_serial_inited = true;
        if (! adb_serial.isEmpty()) {
            __adb_serial = adb_serial_buffer.data();
        }
    }
    isOK = true;
    adbSock.connectToHost("127.0.0.1", 5037, QIODevice::ReadWrite);
    adbSock.waitForConnected();
}

void AdbClient::setAdbSerial(const QString& ser)
{
    static QByteArray adb_serial_buffer;
    adb_serial_buffer = ser.toUtf8();
    __adb_serial = adb_serial_buffer.data();
    qDebug() << "set adb serial to" << __adb_serial;
    AdbClient::doAdbKill();
}

QString AdbClient::getAdbSerial()
{
    if (__adb_serial) {
        return QString((const char*) __adb_serial);
    }
    return "";
}

AdbClient::~AdbClient()
{
    adbSock.close();
}

volatile const char* __adb_serial = NULL;

bool AdbClient::readx(void* data, qint64 max)
{
    int done = 0;
    while (max > done) {
        int n = adbSock.read((char*)data + done, max - done);
        if (n < 0) {
            qDebug() << "adb read wait failed";
            return false;
        } else if (n == 0) {
            if (!adbSock.waitForReadyRead(-1)) {
                qDebug() << "adb read ended prematurely, done: " << done << "max: " << max;
                return false;
            }
        }
        done += n;
    }
    return true;
}

bool _writex(QIODevice& io, const void* data, qint64 max)
{
    int done = 0;
    while (max > done) {
        int n = io.write((char*)data + done, max - done);
        if (n <= 0) {
            return false;
        }
        done += n;
    }
    return true;

}

bool AdbClient::writex(const void* data, qint64 max)
{
    return _writex(adbSock, data, max);
}

void AdbClient::adb_close()
{
    adbSock.close();
}

bool AdbClient::adb_status()
{
    char buf[5];
    unsigned len;

    adbSock.flush();

    if(!readx(buf, 4)) {
        __adb_error = "protocol fault (no status)";
        return false;
    }

    if(!memcmp(buf, "OKAY", 4)) {
        return true;
    }

    if(memcmp(buf, "FAIL", 4)) {
        __adb_error.sprintf(
            "protocol fault (status %02x %02x %02x %02x?!)",
            buf[0], buf[1], buf[2], buf[3]);
        return false;
    }

    if(!readx(buf, 4)) {
        __adb_error = "protocol fault (status len)";
        return false;
    }

    buf[4] = 0;
    len = strtoul((char*)buf, 0, 16);
    if(len > 255) len = 255;

    char buf2[256];
    if(!readx(buf2, len)) {
        __adb_error = "protocol fault (status read)";
        return false;
    }
    buf2[len] = 0;
    __adb_error = buf2;
    return false;
}

bool AdbClient::switch_socket_transport()
{
    char service[64];
    char tmp[5];
    int len;

    if (__adb_serial) {
        snprintf(service, sizeof service, "host:transport:%s", __adb_serial);
    } else {
        snprintf(service, sizeof service, "host:%s", "transport-any");
    }
    len = strlen(service);
    snprintf(tmp, sizeof tmp, "%04x", len);

    if(!writex(tmp, 4) || !writex(service, len)) {
        __adb_error = "write failure during connection";
        return false;
    }

    return adb_status();
}

bool AdbClient::adb_connect(const char *service)
{
    char tmp[5];
    int len;

    len = strlen(service);
    if((len < 1) || (len > 1024)) {
        __adb_error = "service name too long";
        return false;
    }
    snprintf(tmp, sizeof tmp, "%04x", len);

    if (memcmp(service,"host",4) != 0 && !switch_socket_transport()) {
        return false;
    }

    if(!writex(tmp, 4) || !writex(service, len)) {
        __adb_error = "write failure during connection";
        adb_close();
        return false;
    }

    return adb_status();
}

AdbClient* AdbClient::doAdbPipe(const QString& cmdLine)
{
    return AdbClient::doAdbPipe(QStringList(cmdLine));
}

AdbClient* AdbClient::doAdbPipe(const QStringList& cmdAndArgs)
{
    AdbClient *adb = new AdbClient();
    QString cmdLine = "shell:";
    foreach(const QString& a, cmdAndArgs) {
        cmdLine += a + " ";
    }

    bool res = adb->adb_connect(cmdLine.toUtf8().constData());
    if (!res) {
        adb->isOK = false;
    }
    return adb;
}

QString AdbClient::doAdbShell(const QStringList& cmdAndArgs)
{
    QStringList res = doAdbShellWithStderr(cmdAndArgs);
    if (res.length() >= 1) {
        return res[0];
    }
    return "";
}

QStringList AdbClient::doAdbShellWithStderr(const QStringList& cmdAndArgs)
{
    AdbClient *adb = doAdbPipe(cmdAndArgs);
    QStringList res;
    if (!adb->isOK) {
        res << "" << adb->__adb_error;
    } else {
        QByteArray buf;

        while (adb->adbSock.waitForReadyRead(-1)) {
            buf += adb->adbSock.readAll();
        }

        QString ret = QString::fromUtf8(buf);
        ret.replace("\r", "");
        while (ret.endsWith("\n")) {
            ret.chop(1);
        }
        res << ret;
    }

    delete adb;
    return res;
}

QString AdbClient::doAdbShell(const QString& cmdLine) {
    return AdbClient::doAdbShell(QStringList(cmdLine));
}

bool AdbClient::sync_recv(const QString& rpath, const QString& lpath)
{
    syncmsg msg;
    int len;
    QFile lfile(lpath);
    char *buffer = send_buffer.data;
    unsigned id;

    len = rpath.toUtf8().size();
    if(len > 1024) return false;


    msg.req.id = ID_RECV;
    msg.req.namelen = htoll(len);
    if(!writex(&msg.req, sizeof(msg.req)) ||
       !writex(rpath.toUtf8(), len)) {
        return false;
    }

    if(!readx(&msg.data, sizeof(msg.data))) {
        return false;
    }
    id = msg.data.id;

    if((id == ID_DATA) || (id == ID_DONE)) {
        lfile.remove();
        QDir dir = QFileInfo(lpath).dir();
        dir.mkpath(dir.path());

        if (!lfile.open(QIODevice::WriteOnly)) {
            qDebug() << "Can't open" << lpath;
            return false;
        }
        goto handle_data;
    } else {
        goto remote_error;
    }

    for(;;) {
        if(!readx(&msg.data, sizeof(msg.data))) {
            return false;
        }
        id = msg.data.id;

    handle_data:
        len = ltohl(msg.data.size);
        if(id == ID_DONE) break;
        if(id != ID_DATA) goto remote_error;
        if(len > SYNC_DATA_MAX) {
            qDebug() << "data overrun\n";
            lfile.close();
            return false;
        }

        if(!readx(buffer, len)) {
            lfile.close();
            return false;
        }

        if(!_writex(lfile, buffer, len)) {
            qDebug() << "cannot write" << lpath;
            lfile.close();
            return false;
        }
    }

    lfile.close();
    return true;

remote_error:
    lfile.close();
    lfile.remove();

    if(id == ID_FAIL) {
        len = ltohl(msg.data.size);
        if(len > 256) len = 256;
        if(!readx(buffer, len)) {
            return false;
        }
        buffer[len] = 0;
    } else {
        memcpy(buffer, &id, 4);
        buffer[4] = 0;
//        strcpy(buffer,"unknown reason");
    }
    qDebug() <<  "failed to copy" << rpath << "to" << lpath << ":" << buffer;
    return false;
}

bool AdbClient::do_sync_pull(const char *rpath, const char *lpath)
{
    unsigned mode;
    if (!adb_connect("sync:")) {
        qDebug() << "error: " << adb_error();
        return false;
    }

    if (!sync_readmode(rpath, &mode)) {
        return false;
    }
    if(mode == 0) {
        qDebug() << "remote object" << rpath << "does not exist\n";
        return false;
    }

    if(S_ISREG(mode) || S_ISLNK(mode) || S_ISCHR(mode) || S_ISBLK(mode)) {
        QString finalLpath = lpath;
        QFileInfo lInfo(lpath);

        if (lInfo.isDir()) {
            finalLpath += "/" + QFileInfo(rpath).fileName();
        }

        if (!sync_recv(rpath, finalLpath)) {
            return false;
        } else {
            sync_quit();
            return true;
        }
    } else if(S_ISDIR(mode)) {
        qDebug() << "Error: do not support directroy: " << rpath;
        return false;
    } else {
        qDebug() << rpath << " is not a file or directroy";
        return false;
    }
}

bool AdbClient::sync_readmode(const char *path, quint32 *mode)
{
    syncmsg msg;
    int len = strlen(path);

    msg.req.id = ID_STAT;
    msg.req.namelen = htoll(len);

    if(!writex(&msg.req, sizeof(msg.req)) ||
       !writex(path, len)) {
        return false;
    }

    if(!readx(&msg.stat, sizeof(msg.stat))) {
        return false;
    }

    if(msg.stat.id != ID_STAT) {
        return false;
    }

    *mode = ltohl(msg.stat.mode);
    return true;
}

bool AdbClient::write_data_file(const QString& path, syncsendbuf *sbuf)
{
    int isOK = true;
    unsigned long long size = 0;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Can't open" << path;
        return false;
    }

    sbuf->id = ID_DATA;
    for(;;) {
        int ret;

        ret = file.read(sbuf->data, SYNC_DATA_MAX);
        if(!ret)
            break;

        if(ret < 0) {
            break;
        }

        sbuf->size = htoll(ret);
        if(!writex(sbuf, sizeof(unsigned) * 2 + ret)){
            isOK = false;
            break;
        }
    }

    file.close();
    return isOK;
}

bool AdbClient::sync_send(const QString& lpath, const QString& rpath,
                          unsigned mtime, quint32 mode)

{
    syncmsg msg;
    int len, r;
    syncsendbuf *sbuf = &send_buffer;
    char tmp[64];

    len = rpath.toUtf8().size();
    if(len > 1024) goto fail;

    snprintf(tmp, sizeof(tmp), ",%d", mode);
    r = strlen(tmp);

    msg.req.id = ID_SEND;
    msg.req.namelen = htoll(len + r);

    if(!writex(&msg.req, sizeof(msg.req)) ||
       !writex(rpath.toUtf8(), len) || !writex(tmp, r)) {
        goto fail;
    }

    write_data_file(lpath, sbuf);

    msg.data.id = ID_DONE;
    msg.data.size = htoll(mtime);
    if(!writex(&msg.data, sizeof(msg.data)))
        goto fail;

    if(!readx(&msg.status, sizeof(msg.status)))
        return -1;

    if(msg.status.id != ID_OKAY) {
        if(msg.status.id == ID_FAIL) {
            len = ltohl(msg.status.msglen);
            if(len > 256) len = 256;
            if(!readx(sbuf->data, len)) {
                return false;
            }
            sbuf->data[len] = 0;
        } else
            strcpy(sbuf->data, "unknown reason");

        fprintf(stderr,"failed to copy '%s' to '%s': %s\n", lpath.toUtf8().constData(), rpath.toUtf8().constData(), sbuf->data);
        return false;
    }

    return true;

fail:
    fprintf(stderr,"protocol failure\n");
    return false;
}

void AdbClient::sync_quit()
{
    syncmsg msg;

    msg.req.id = ID_QUIT;
    msg.req.namelen = 0;

    writex(&msg.req, sizeof(msg.req));
}

bool AdbClient::do_sync_push(const char *lpath, const char *rpath)
{
    quint32 mode;
    int fd;

    if (!adb_connect("sync:")) {
        return false;
    }

    QFileInfo lInfo(lpath);
    if (lInfo.isDir()) {
        qDebug() << lpath << "is a directory, not supported for push";
        return false;
    } else {
        if(!sync_readmode(rpath, &mode)) {
            qDebug() << "can't read rpath mode";
            return false;
        }
        QString finalRemotePath = rpath;
        if((mode != 0) && S_ISDIR(mode)) {
                /* if we're copying a local file to a remote directory,
                ** we *really* want to copy to remotedir + "/" + localfilename
                */
            QString name = lInfo.fileName();
            finalRemotePath += "/" + name;
        }

        if(!sync_send(lpath, finalRemotePath, lInfo.lastModified().toTime_t(), 0x81b6)) {
            qDebug() << "send failed";
            return false;
        } else {
            sync_quit();
            return true;
        }
    }

    return true;
}

bool AdbClient::doAdbPush(const QString& lpath, const QString& rpath)
{
    AdbClient *adb = new AdbClient();
    bool res = adb->do_sync_push(lpath.toUtf8().constData(), rpath.toUtf8().constData());
    delete adb;
    return res;
}

bool AdbClient::doAdbPull(const QString& rpath, const QString& lpath)
{
    AdbClient *adb = new AdbClient();
    bool res = adb->do_sync_pull(rpath.toUtf8().constData(), lpath.toUtf8().constData());
    delete adb;
    return res;
}

int AdbClient::doAdbKill()
{
    AdbClient *adb = new AdbClient();
    adb->adbSock.write("0009host:kill");
    adb->adbSock.flush();

    while (adb->adbSock.waitForReadyRead()) {
        adb->adbSock.readAll();
    }

    adb->adbSock.close();
    delete adb;
    return 0;
}

QStringList AdbClient::doAdbDevices()
{
    AdbClient *adb = new AdbClient();
    bool res = adb->adb_connect("host:devices");
    QByteArray buf(5, 0);
    QString devicesStr;
    if (res) {
        if (adb->readx(buf.data(), 4)) {
            int len = strtoul(buf.data(), 0, 16);
            buf.reserve(len + 1);
            buf[len] = 0;
            adb->readx(buf.data(), len);
        } else {
            buf.fill(0);
        }
        devicesStr = QString::fromUtf8(buf);
    } else {
        qDebug() << "adb devices not ok:" << adb->__adb_error;
    }
    delete adb;
    QStringList ret;
    foreach(const QString& line, devicesStr.split("\n", QString::SkipEmptyParts)) {
        if (line.contains("\t")) {
            foreach(const QString& dev, line.split("\t")) {
                ret << dev;
                break;
            }
        }
    }
    return ret;
}

// "host:forward:tcp:28888;localabstract:Wrench"
int AdbClient::doAdbForward(const QString& forwardSpec)
{
    AdbClient *adb = new AdbClient();
    QString instanceString = QProcessEnvironment::systemEnvironment().value("WRENCH_INSTANCE", "0");

    QString forwardSpecVar = forwardSpec;
    if (instanceString != "0") {
        QRegExp re("(.*forward:tcp:)(\\d+)(;.*)");
        if (re.exactMatch(forwardSpec)) {
            QString port = re.cap(2);
            forwardSpecVar = re.cap(1) + QString::asprintf("%d", port.toInt() + instanceString.toInt()) + re.cap(3);
            qDebug() << "forwardSpecVar is " << forwardSpecVar;
        }
    }
    adb->adb_connect(forwardSpecVar.toUtf8().constData());
    delete adb;
    return 0;
}
