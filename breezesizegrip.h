#pragma once

/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>     *
 * Copyright (C) 2023 by Paulo Otávio de Lima <dpaulootavio5@outlook.com> *
 *                                                                        *
 * This program is free software; you can redistribute it and/or modify   *
 * it under the terms of the GNU General Public License as published by   *
 * the Free Software Foundation; either version 2 of the License, or      *
 * (at your option) any later version.                                    *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program; if not, write to the                          *
 * Free Software Foundation, Inc.,                                        *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .         *
 *************************************************************************/

#include "breezedecoration.h"
#include "config-breeze.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>
#include <QPointer>

#if BREEZE_HAVE_X11
#include <xcb/xcb.h>
#endif

namespace Breeze
{

    /// Implements size grip for all widgets
    class SizeGrip: public QWidget
    {

        Q_OBJECT

    public:

        //* constructor
        explicit SizeGrip( Decoration* );

        //* constructor
        virtual ~SizeGrip();

        protected Q_SLOTS:

        /// Uupdate background color
        void updateActiveState();

        /// Update position
        void updatePosition();

        /// Embed into parent widget
        void embed();

    protected:

        // Event Handlers
        //@{

        /// Paint
        virtual void paintEvent( QPaintEvent* ) override;

        /// Mouse Press
        virtual void mousePressEvent( QMouseEvent* ) override;

        //@}

    private:

        /// Send resize event
        void sendMoveResizeEvent( QPoint );

        /// Grip Size
        enum {
            Offset = 0,
            GripSize = 14
        };

        /// Decoration
        QPointer<Decoration> m_decoration;

        /// Move/Resize atom
        #if BREEZE_HAVE_X11
        xcb_atom_t m_moveResizeAtom = 0;
        #endif

    };


}
