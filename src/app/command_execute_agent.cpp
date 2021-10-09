#include "base/command.h"

#include <QString>
#include <QTemporaryFile>
#include <QProcess>
#include <QDebug>

int main (int argc, char* argv[]) {
    if (argc != 3) {
        qInfo() << "Arguments error\n";
        return 1;
    }

    QString password = argv[1];
    QString cmd = argv[2];

    QString script = "#!/bin/bash -f\n"
            "KEY=$1\n"
            "PASSWORD=$2\n"
            "PAIN=$(echo ${PASSWORD} | openssl aes128 -d -k ${KEY} -base64 2>/dev/null)\n"
            + cmd.arg("${PAIN}") + "\n";

    QTemporaryFile tmp_script;
    if (tmp_script.open()) {
        tmp_script.write(script.toUtf8());
        tmp_script.close();
    }

    if (!installer::SpawnCmd("/bin/bash", {tmp_script.fileName(), "uos@123!!", password})) {
        qCritical() << "Script run failed";
        return 1;
    }

    return 0;
}
