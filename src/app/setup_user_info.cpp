#include <QString>
#include <QTemporaryFile>
#include <QProcess>
#include <QDebug>

int main (int argc, char* argv[]) {

    if (argc != 3) {
        qDebug() << "Usage: command user_name password";
        return 0;
    }

    QString script = "#!/bin/bash -f\n"
            "USER_NAME=$1\n"
            "PASSWORD=$2\n"
            "KEY=$3\n"
            "PAIN=$(echo ${PASSWORD} | openssl aes128 -d -k ${KEY} -base64 2>/dev/null)\n"
            "echo \"${USER_NAME}:${PAIN}\" | chpasswd\n";

    QString user_name = QString(argv[1]);
    QString password = QString(argv[2]);

    QTemporaryFile tmp_script;
    if (tmp_script.open()) {
        tmp_script.write(script.toUtf8());
        tmp_script.close();
    }

    QProcess p;
    p.execute("/bin/bash", {tmp_script.fileName(), user_name, password, "uos@123!!" });
    return 0;
}
