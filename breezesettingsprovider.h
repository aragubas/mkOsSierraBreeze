#pragma once
/*
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

#include "breezedecoration.h"
#include "breezesettings.h"
#include "breeze.h"

#include <KSharedConfig>

#include <QObject>

namespace Breeze
{

    class SettingsProvider: public QObject
    {

        Q_OBJECT

    public:

        ~SettingsProvider();

        /// Returns instance of singleton
        static SettingsProvider *self();

        /// Internal settings for given decoration
        InternalSettingsPtr internalSettings(Decoration *) const;

    public Q_SLOTS:

        /// Reconfigure
        void reconfigure();

    private:

        SettingsProvider();

        /// Default Configuration
        InternalSettingsPtr m_defaultSettings;

        /// Exceptions
        InternalSettingsList m_exceptions;

        /// Configuration Object
        KSharedConfigPtr m_config;

        /// Singleton Instance
        static SettingsProvider *s_self;

    };

}
