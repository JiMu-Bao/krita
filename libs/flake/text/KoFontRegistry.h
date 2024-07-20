/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFONTREGISTRY_H
#define KOFONTREGISTRY_H

#include <QScopedPointer>
#include <QVector>

#include <KoFontLibraryResourceUtils.h>
#include <KoFFWWSConverter.h>

#include "kritaflake_export.h"

/**
 * @brief The KoFontRegistry class
 * A wrapper around a freetype library.
 *
 * This class abstract away loading freetype faces from css
 * values. Internally it uses fontconfig to get the filename
 * and then loads the freetype face within a mutex-lock.
 *
 * It also provides a configuration function to handle all the
 * size and variation axis values.
 */
class KRITAFLAKE_EXPORT KoFontRegistry
{
public:
    KoFontRegistry();
    ~KoFontRegistry();

    static KoFontRegistry *instance();

    /**
     * @brief facesForCSSValues
     * This selects a font with fontconfig using the given
     * values. If "text" is not empty, it will try to select
     * fallback fonts as well.
     *
     * @returns a vector of loaded FT_Faces, the "lengths" vector
     * will be filled with the lengths of consequetive characters
     * a face can be set on.
     */
    std::vector<FT_FaceSP> facesForCSSValues(const QStringList &families,
                                             QVector<int> &lengths,
                                             const QMap<QString, qreal> &axisSettings,
                                             const QString &text = "",
                                             quint32 xRes = 72,
                                             quint32 yRes = 72,
                                             qreal size = -1,
                                             qreal fontSizeAdjust = 1.0,
                                             int weight = 400,
                                             int width = 100,
                                             int slantMode = false,
                                             int slantValue = 0,
                                             const QString &language = QString());

    /**
     * @brief configureFaces
     * This configures a list of faces with pointSize and
     * variation settings.
     *
     * Sometimes setting the size can get quite tricky (like with
     * bitmap fonts), so this convenience function handles all that.
     *
     * @returns whether the configuration was successful.
     */
    bool configureFaces(const std::vector<FT_FaceSP> &faces,
                        qreal size,
                        qreal fontSizeAdjust,
                        quint32 xRes,
                        quint32 yRes,
                        const QMap<QString, qreal> &axisSettings);

    /**
     * @brief collectRepresentations
     * @return a list of Width/Weight/Slant font family representations.
     */
    QList<KoFontFamilyWWSRepresentation> collectRepresentations() const;

    /**
     * @brief representationByFamilyName
     * This simplifies retrieving the representation for a given font family.
     * @param familyName - the familyName associated with the font.
     * @param found - bool to check for success.
     * @return the font family.
     */
    KoFontFamilyWWSRepresentation representationByFamilyName(const QString &familyName, bool *found = nullptr) const;

    /**
     * Update the config and reset the FontChangeListener.
     */
    void updateConfig();

private:
    class Private;

    friend class TestSvgText;
    friend class SvgTextCursorTest;

    /**
     * @brief addFontFilePathToRegistery
     * This adds a font file to the registery. Right now only used by unittests.
     *
     * @param path the path of the font file.
     * @return Whether adding the font file was succesful.
     */
    bool addFontFilePathToRegistery(const QString &path);

    /**
     * @brief addFontFileDirectoryToRegistery
     * This adds a directory of font files to the registery. Right now only used by unittests.
     * @param path the path of the directory.
     * @return whether it was succesful.
     */
    bool addFontFileDirectoryToRegistery(const QString &path);

    QScopedPointer<Private> d;

    Q_DISABLE_COPY(KoFontRegistry);
};

#endif // KOFONTREGISTRY_H
