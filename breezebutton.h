#pragma once

/*
* Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
* Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
* Copyright 2023  Paulo Otávio de Lima (aka Aragubas) <dpaulootavio5@outlook.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <KDecoration2/DecorationButton>
#include "breezedecoration.h"

#include <QHash>
#include <QImage>

#include <QVariantAnimation>

class QVariantAnimation;

namespace Breeze
{

    class Button : public KDecoration2::DecorationButton
    {
        Q_OBJECT

    public:

        //* constructor
        explicit Button(QObject *parent, const QVariantList &args);

        //* destructor
        virtual ~Button() = default;

        //* button creation
        static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

        //* render
        virtual void paint(QPainter *painter, const QRect &repaintRegion) override;

        //* flag
        enum Flag {
            FlagNone,
            FlagStandalone,
            FlagFirstInList,
            FlagLastInList
        };

        //* flag
        void setFlag( Flag value ) { m_flag = value; }

        //* standalone buttons
        bool isStandAlone() const { return m_flag == FlagStandalone; }

        //* set icon size
        void setIconSize( const QSize& value ) { m_iconSize = value; }

        //*@name active state change animation
        //@{
        void setOpacity( qreal value ) {
            if( m_opacity == value ) return;
            m_opacity = value;
            update();
        }

        qreal opacity() const { return m_opacity; }

        //@}

    private:

        //* private constructor
        explicit Button(KDecoration2::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

        /// Draw window buttons
        void drawButtonbackground(QPainter*, QColor button_color, QColor titleBarColor) const;
        void drawWindowButtons(QPainter*) const;

        //*@name colors
        //@{
        QColor fontColor() const;
        QColor foregroundColor() const;
        QColor backgroundColor() const;
        QColor mixColors(const QColor&, const QColor&, qreal) const;
        QColor autoColor( const bool, const bool, const bool, const QColor, const QColor ) const;
        //@}

        //*@hover buttons
        //@{
        bool hovered() const;
        //@}

        Flag m_flag = FlagNone;

        //* icon size
        QSize m_iconSize;

        //* active state change opacity
        qreal m_opacity = 0;
    };

}
