/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef GLYPHPALETTEPROXYMODEL_H
#define GLYPHPALETTEPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <KoFontGlyphModel.h>
#include <QChar>

class GlyphPaletteProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    GlyphPaletteProxyModel(QObject *parent = nullptr);
    ~GlyphPaletteProxyModel() override;

    int filter() const;

    QMap<int, QString> filterLabels();

public Q_SLOTS:
    void setFilter(int filter);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
private:
    int m_filter{0};

};

#endif // GLYPHPALETTEPROXYMODEL_H
