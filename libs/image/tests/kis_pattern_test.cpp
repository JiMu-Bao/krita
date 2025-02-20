/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pattern_test.h"


#include <simpletest.h>
#include <resources/KoPattern.h>

#include <QStandardPaths>
#include <QCryptographicHash>
#include <QByteArray>

#include <KoMD5Generator.h>
#include <kis_debug.h>
#include <KisGlobalResourcesInterface.h>

#include <testutil.h>

void KoPatternTest::testCreation()
{
    KoPattern test(QString(FILES_DATA_DIR) + '/' + "pattern.pat");
    test.load(KisGlobalResourcesInterface::instance());
    QVERIFY(test.name() == "三");
    QVERIFY(!test.pattern().isNull());
}

void KoPatternTest::testRoundTripMd5()
{
    QString filename(QString(FILES_DATA_DIR) + '/' + "test_pattern.png");
    QString patFilename("test_pattern.pat");

    KoPattern pngPattern(filename);
    QVERIFY(pngPattern.load(KisGlobalResourcesInterface::instance()));
    pngPattern.setMD5Sum(KoMD5Generator::generateHash(filename));

    qDebug() << "PNG Name:" << pngPattern.name();
    qDebug() << "PNG Filename:" << pngPattern.filename();

    pngPattern.setFilename(patFilename);
    QVERIFY(pngPattern.save());

    KoPattern patPattern(patFilename);
    QVERIFY(patPattern.load(KisGlobalResourcesInterface::instance()));
    patPattern.setMD5Sum(KoMD5Generator::generateHash(patFilename));

    qDebug() << "PAT Name:" << patPattern.name();
    qDebug() << "PAT Filename:" << patPattern.filename();

    dbgKrita << pngPattern.pattern().format();
    dbgKrita << patPattern.pattern().format();

    QImage im1 = pngPattern.pattern().convertToFormat(QImage::Format_ARGB32);
    QImage im2 = patPattern.pattern().convertToFormat(QImage::Format_ARGB32);

    QCOMPARE(im1, im2);

    QCryptographicHash h1(QCryptographicHash::Md5);
    h1.addData(QByteArray::fromRawData((const char*)im1.constBits(), im1.sizeInBytes()));

    QCryptographicHash h2(QCryptographicHash::Md5);
    h2.addData(QByteArray::fromRawData((const char*)im2.constBits(), im2.sizeInBytes()));

    // Compares the images: they should be the same
    QCOMPARE(h1.result(), h2.result());

    // Compares the md5sum taken from the file: they should be different
    QVERIFY(pngPattern.md5Sum() != patPattern.md5Sum());
}


SIMPLE_TEST_MAIN(KoPatternTest)
