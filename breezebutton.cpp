/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
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
#include "breezebutton.h"

#include <KDecoration2/DecoratedClient>
#include <KColorUtils>
#include <KIconLoader>

#include <QPainter>
#include <QPainterPath>

namespace Breeze
{

    using KDecoration2::ColorRole;
    using KDecoration2::ColorGroup;
    using KDecoration2::DecorationButtonType;


    Button::Button(DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
        , m_animation( new QVariantAnimation( this ) )
    {

        // setup animation
        // It is important start and end value are of the same type, hence 0.0 and not just 0
        m_animation->setStartValue( 0.0 );
        m_animation->setEndValue( 1.0 );
        // Linear to have the same easing as Breeze animations
        m_animation->setEasingCurve( QEasingCurve::Linear );
        connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setOpacity(value.toReal());
        });

        // setup default geometry
        const int height = decoration->buttonHeight();
        setGeometry(QRect(0, 0, height, height));
        setIconSize(QSize( height, height ));

        // connections
        connect(decoration->client().toStrongRef().data(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
        connect(decoration->settings().data(), &KDecoration2::DecorationSettings::reconfigured, this, &Button::reconfigure);
        connect( this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState );

        if (decoration->objectName() == "applet-window-buttons") {
            connect( this, &Button::hoveredChanged, [=](bool hovered){
                    decoration->setButtonHovered(hovered);
                    });
        }
        connect(decoration, SIGNAL(buttonHoveredChanged()), this, SLOT(update()));

        reconfigure();

    }

    Button::Button(QObject *parent, const QVariantList &args)
        : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
    {
        m_flag = FlagStandalone;
        //! icon size must return to !valid because it was altered from the default constructor,
        //! in Standalone mode the button is not using the decoration metrics but its geometry
        m_iconSize = QSize(-1, -1);
    }

    Button *Button::create(DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
    {
        if (auto d = qobject_cast<Decoration*>(decoration))
        {
            Button *b = new Button(type, d, parent);
            switch( type )
            {

                case DecorationButtonType::Close:
                b->setVisible( d->client().toStrongRef().data()->isCloseable() );
                QObject::connect(d->client().toStrongRef().data(), &KDecoration2::DecoratedClient::closeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Maximize:
                b->setVisible( d->client().toStrongRef().data()->isMaximizeable() );
                QObject::connect(d->client().toStrongRef().data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Minimize:
                b->setVisible( d->client().toStrongRef().data()->isMinimizeable() );
                QObject::connect(d->client().toStrongRef().data(), &KDecoration2::DecoratedClient::minimizeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::ContextHelp:
                b->setVisible( d->client().toStrongRef().data()->providesContextHelp() );
                QObject::connect(d->client().toStrongRef().data(), &KDecoration2::DecoratedClient::providesContextHelpChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Shade:
                b->setVisible( d->client().toStrongRef().data()->isShadeable() );
                QObject::connect(d->client().toStrongRef().data(), &KDecoration2::DecoratedClient::shadeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Menu:
                QObject::connect(d->client().toStrongRef().data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() { b->update(); });
                break;

                default: break;
            }

            return b;
        }

        return nullptr;

    }

    void Button::paint(QPainter *painter, const QRect &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        if (!decoration()) return;

        painter->save();

        // translate from offset
        if( m_flag == FlagFirstInList ) painter->translate( m_offset );
        else painter->translate( 0, m_offset.y() );

        if( !m_iconSize.isValid() || isStandAlone() ) m_iconSize = geometry().size().toSize();

        // Menu Button
        if (type() == DecorationButtonType::Menu) {
            const QRectF iconRect( geometry().topLeft(), 0.8*m_iconSize );
            const qreal width( m_iconSize.width() );

            painter->translate( 0.1*width, 0.1*width );

            if (auto deco = qobject_cast<Decoration*>(decoration())) {
              const QPalette activePalette = KIconLoader::global()->customPalette();
              QPalette palette = decoration()->client().toStrongRef().data()->palette();
              palette.setColor(QPalette::Foreground, deco->fontColor());
              KIconLoader::global()->setCustomPalette(palette);
              decoration()->client().toStrongRef().data()->icon().paint(painter, iconRect.toRect());
              if (activePalette == QPalette()) {
                KIconLoader::global()->resetPalette();
              }    else {
                KIconLoader::global()->setCustomPalette(palette);
              }
            } else {
              decoration()->client().toStrongRef().data()->icon().paint(painter, iconRect.toRect());
            }

        } else {
            drawWindowButtons( painter );
        }

        painter->restore();

    }

    void Button::drawButtonbackground(QPainter* painter, QColor backgroundColor) const {
      painter->setBrush(backgroundColor);

      // Translates to the center
      QPoint centerPoint = QPoint(m_iconSize.width() / 2, m_iconSize.height() / 2);
      painter->translate(centerPoint);

      // Draws the actual ellipse
      qreal radius = this->buttonRadius();
      painter->drawEllipse(QPointF(0, 0), radius, radius);

      // Resets Brush
      painter->setBrush(Qt::NoBrush);

      // Undo the translation
      painter->translate(-centerPoint);
    }

    void Button::drawWindowButtons( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        // Translates to the topLeft corner of the bounding box
        // This will be the base translation for all drawing
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        auto d = qobject_cast<Decoration*>( decoration() );
        painter->scale(0.8, 0.8);
        painter->translate(4, 4); // TODO: Calculate scaling offset

        bool inactiveWindow( d && !d->client().toStrongRef().data()->isActive() );
        bool isMatchTitleBarColor( d && d->internalSettings()->matchColorForTitleBar() );

        QColor darkSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(250, 251, 252) : QColor(34, 45, 50) );
        QColor lightSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(192, 193, 194) : QColor(250, 251, 252) );

        QColor titleBarColor (d->titleBarColor());

        // symbols pen
        QColor symbolColor = darkSymbolColor;
        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        symbol_pen.setWidthF( 9./7.*1.7*qMax((qreal)1.0, 20/width ) );

        int titlebarColorGrayness = qGray(titleBarColor.rgb());
        int margin = 7;

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor button_color;
                if ( !inactiveWindow && titlebarColorGrayness < 128 ) {
                  button_color = QColor(238, 102, 90);
                } else if ( !inactiveWindow ) {
                  button_color = QColor(255, 97, 89);
                } else if ( titlebarColorGrayness < 128 ) {
                  button_color = QColor(100, 100, 100);
                } else {
                  button_color = QColor(200, 200, 200);
                }

                QPen button_pen( titlebarColorGrayness < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                  button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                  button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setPen( button_pen );

                drawButtonbackground(painter, button_color);

                if ( this->hovered() )
                {
                  // Hardcoded color since I don't know the original color
                  // and the multiplication mode
                  symbol_pen.setColor(QColor(77, 0, 0));
                  painter->setPen( symbol_pen );

                  // Draws an X shape
                  int bottomPoint = m_iconSize.height() - margin;
                  int rightPoint = m_iconSize.width() - margin;
                  painter->drawLine( QPointF( margin, margin ), QPointF( rightPoint, bottomPoint ) );
                  painter->drawLine( QPointF( margin, bottomPoint ), QPointF( rightPoint, margin ) );
                }
                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color;
                if ( !inactiveWindow && titlebarColorGrayness < 128 )
                  button_color = QColor(100, 196, 86);
                else if( !inactiveWindow )
                  button_color = QColor(41, 204, 65);
                else if ( titlebarColorGrayness < 128 )
                  button_color = QColor(100, 100, 100);
                else
                  button_color = QColor(200, 200, 200);
                QPen button_pen( titlebarColorGrayness < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                  button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                  button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setPen( button_pen );

                drawButtonbackground(painter, button_color);

                if ( this->hovered() )
                {
                  painter->setPen( Qt::NoPen );

                  // two triangles
                  QPainterPath path1, path2;
                  if( isChecked() )
                  {
                      QPoint centerPoint = QPoint(m_iconSize.width() / 2, m_iconSize.height() / 2);
                      // int marginBase = margin / 1.2;
                      // int lowestPoint = m_iconSize.height() - marginBase;
                      // path1.moveTo(centerPoint.x(), centerPoint.y()); // Base Point
                      // path1.lineTo(marginBase, m_iconSize.height() / 2); // Left?
                      // path1.lineTo(marginBase + 8, lowestPoint);
                      //
                      // path2.moveTo(centerPoint.x(), centerPoint.y()); // Base Point
                      // path2.lineTo(15.5, 8.5); // Left?
                      // path2.lineTo(9.5, 2.5);
                      int marginBase = margin / 2;
                      int lowestPoint = m_iconSize.height() - marginBase;
                      double slice = 0.5;
                      path1.moveTo(centerPoint.x() - slice, centerPoint.y() + slice);
                      path1.lineTo(marginBase, centerPoint.y());
                      path1.lineTo(centerPoint.x(), lowestPoint);

                      path2.moveTo(centerPoint.x() + slice, centerPoint.y() - slice);
                      path2.lineTo(centerPoint.x() + slice, marginBase); // Left Most Point
                      path2.lineTo(m_iconSize.width() - marginBase, centerPoint.y() - slice); // Right most point


                  }
                  else
                  {
                      int marginBase = margin / 1.25;
                      int lowestPoint = m_iconSize.height() - marginBase;
                      path1.moveTo(marginBase, lowestPoint);
                      path1.lineTo(marginBase + 8, lowestPoint);
                      path1.lineTo(marginBase, m_iconSize.height() - marginBase - 7);

                      path2.moveTo(m_iconSize.width() - marginBase, marginBase); // Base
                      path2.lineTo(marginBase + 2, marginBase); // Left Most Point
                      path2.lineTo(m_iconSize.width() - marginBase, m_iconSize.height() - marginBase - 2); // Right most point
                  }

                  QColor maximizeForegroundColor = QColor(0, 101, 0); // Hardcoded color since I don't know the origina color and the mult mode used'

                  painter->fillPath(path1, QBrush(maximizeForegroundColor));
                  painter->fillPath(path2, QBrush(maximizeForegroundColor));
                  //painter->setBrush(QBrush(QColor(255, 0, 255)));
                  //painter->drawRoundedRect(margin, margin, m_iconSize.width() - margin, m_iconSize.height() - margin, 4, 4);

                }
                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color;
                if ( !inactiveWindow && titlebarColorGrayness < 128 )
                  button_color = QColor(223, 192, 76);
                else if( !inactiveWindow )
                  button_color = QColor(255, 193, 46);
                else if ( titlebarColorGrayness < 128 )
                  button_color = QColor(100, 100, 100);
                else
                  button_color = QColor(200, 200, 200);
                QPen button_pen( titlebarColorGrayness < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                  button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                  button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setPen( button_pen );

                drawButtonbackground(painter, button_color);

                if ( this->hovered() )
                {
                  symbol_pen.setColor(QColor(153, 87, 0));
                  painter->setPen( symbol_pen );
                  int centerY = m_iconSize.height() / 2;
                  int marginBase = margin / 1.2;
                  painter->drawLine( QPointF(marginBase, centerY ), QPointF( m_iconSize.width() - marginBase, centerY ) );
                }
                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color;
                if ( !inactiveWindow )
                  button_color = QColor(125, 209, 200);
                else if ( titlebarColorGrayness < 128 )
                  button_color = QColor(100, 100, 100);
                else
                  button_color = QColor(200, 200, 200);
                QPen button_pen( titlebarColorGrayness < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                  button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                  button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setPen( button_pen );

                drawButtonbackground(painter, button_color);

                if ( this->hovered() || isChecked() )
                {
                  painter->setPen( Qt::NoPen );
                  painter->setBrush(QBrush(symbolColor));
                  QPoint centerPoint = QPoint(m_iconSize.width() / 2, m_iconSize.height() / 2);
                  painter->drawEllipse(centerPoint, 3, 3);
                }
                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color;
                if ( !inactiveWindow )
                  button_color = QColor(204, 176, 213);
                else if ( titlebarColorGrayness < 128 )
                  button_color = QColor(100, 100, 100);
                else
                  button_color = QColor(200, 200, 200);
                QPen button_pen( titlebarColorGrayness < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                  button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                  button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setPen( button_pen );

                drawButtonbackground(painter, button_color);

                if ( isChecked() )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 12 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 11);
                    path.lineTo(5, 6);
                    path.lineTo(13, 6);
                    painter->fillPath(path, QBrush(symbolColor));

                }
                else if ( this->hovered() ) {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 6 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 7);
                    path.lineTo(5, 12);
                    path.lineTo(13, 12);
                    painter->fillPath(path, QBrush(symbolColor));
                }
                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color;
                if ( !inactiveWindow )
                  button_color = QColor(255, 137, 241);
                else if ( titlebarColorGrayness < 128 )
                  button_color = QColor(100, 100, 100);
                else
                  button_color = QColor(200, 200, 200);
                QPen button_pen( titlebarColorGrayness < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                  button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                  button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setPen( button_pen );

                drawButtonbackground(painter, button_color);

                if ( this->hovered() || isChecked() )
                {
                  painter->setPen( Qt::NoPen );

                  QPainterPath path;
                  path.moveTo(9, 12);
                  path.lineTo(5, 6);
                  path.lineTo(13, 6);
                  painter->fillPath(path, QBrush(symbolColor));
                }
                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color;
                if ( !inactiveWindow )
                  button_color = QColor(135, 206, 249);
                else if ( titlebarColorGrayness < 128 )
                  button_color = QColor(100, 100, 100);
                else
                  button_color = QColor(200, 200, 200);
                QPen button_pen( titlebarColorGrayness < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                  button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                  button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setPen( button_pen );

                drawButtonbackground(painter, button_color);

                if ( this->hovered() || isChecked() )
                {
                  painter->setPen( Qt::NoPen );

                  QPainterPath path;
                  path.moveTo(9, 6);
                  path.lineTo(5, 12);
                  path.lineTo(13, 12);
                  painter->fillPath(path, QBrush(symbolColor));
                }
                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                QColor menuSymbolColor;
                bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
                if (isSystemForegroundColor)
                  menuSymbolColor = this->fontColor();
                else {
                  uint r = qRed(titleBarColor.rgb());
                  uint g = qGreen(titleBarColor.rgb());
                  uint b = qBlue(titleBarColor.rgb());
                  qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
                  if ( colorConditional > 186 || g > 186 )
                    menuSymbolColor = darkSymbolColor;
                  else
                    menuSymbolColor = lightSymbolColor;
                }

                QPen menuSymbol_pen( menuSymbolColor );
                menuSymbol_pen.setJoinStyle( Qt::MiterJoin );
                menuSymbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

                painter->setPen( menuSymbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color;
                if ( !inactiveWindow )
                  button_color = QColor(102, 156, 246);
                else if ( titlebarColorGrayness < 128 )
                  button_color = QColor(100, 100, 100);
                else
                  button_color = QColor(200, 200, 200);
                QPen button_pen( titlebarColorGrayness < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                  button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                  button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setPen( button_pen );

                drawButtonbackground(painter, button_color);

                if ( this->hovered() || isChecked() )
                {
                  painter->setPen( symbol_pen );
                  QPainterPath path;
                  path.moveTo( 6, 6 );
                  path.arcTo( QRectF( 5.5, 4, 7.5, 4.5 ), 180, -180 );
                  path.cubicTo( QPointF(11, 9), QPointF( 9, 6 ), QPointF( 9, 10 ) );
                  painter->drawPath( path );
                  painter->drawPoint( 9, 13 );
                }
                break;
            }

            default: break;
        }
    }

    // https://stackoverflow.com/questions/25514812/how-to-animate-color-of-qbrush
    QColor Button::mixColors(const QColor &cstart, const QColor &cend, qreal progress) const
    {
        int sh = cstart.hsvHue();
        int eh = cend.hsvHue();
        int ss = cstart.hsvSaturation();
        int es = cend.hsvSaturation();
        int sv = cstart.value();
        int ev = cend.value();
        int hr = qAbs( sh - eh );
        int sr = qAbs( ss - es );
        int vr = qAbs( sv - ev );
        int dirh =  sh > eh ? -1 : 1;
        int dirs =  ss > es ? -1 : 1;
        int dirv =  sv > ev ? -1 : 1;

        return QColor::fromHsv( sh + dirh * progress * hr,
                                ss + dirs * progress * sr,
                                sv + dirv * progress * vr );
    }

    QColor Button::fontColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );

        if( !d ) {

            return QColor();

        } else {

            return d->fontColor();

        }

    }

    QColor Button::foregroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        QColor titleBarColor ( d->titleBarColor() );

        if( !d ) {

            return QColor();

        } else if( isPressed() ) {

            return titleBarColor;

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade ) && isChecked() ) {

            return titleBarColor;

        } else if( m_animation->state() == QAbstractAnimation::Running ) {

            return KColorUtils::mix( d->fontColor(), titleBarColor, m_opacity );

        } else if( this->hovered() ) {

            return titleBarColor;

        } else {

            return d->fontColor();

        }

    }

        //__________________________________________________________________
    QColor Button::backgroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        if( !d ) {

            return QColor();

        }

        auto c = d->client().toStrongRef().data();
        if( isPressed() ) {

            if( type() == DecorationButtonType::Close ) return c->color( ColorGroup::Warning, ColorRole::Foreground );
            else return KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade ) && isChecked() ) {

            return d->fontColor();

        } else if( m_animation->state() == QAbstractAnimation::Running ) {

            if( type() == DecorationButtonType::Close )
            {
                QColor color( c->color( ColorGroup::Warning, ColorRole::Foreground ).lighter() );
                color.setAlpha( color.alpha()*m_opacity );
                return color;

            } else {

                QColor color( d->fontColor() );
                color.setAlpha( color.alpha()*m_opacity );
                return color;

            }

        } else if( this->hovered() ) {

            if( type() == DecorationButtonType::Close ) return c->color( ColorGroup::Warning, ColorRole::Foreground ).lighter();
            else return d->fontColor();

        } else {

            return QColor();

        }

    }

    qreal Button::buttonRadius() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );

        if ( d->internalSettings()->animationsEnabled() && ( !isChecked() || ( isChecked() && type() == DecorationButtonType::Maximize ) ) ) {
          return static_cast<qreal>(7) + static_cast<qreal>(2) * m_animation->currentValue().toReal();
        }
        else
          return static_cast<qreal>(9);
    }


    QColor Button::autoColor( const bool inactiveWindow, const bool useActiveButtonStyle, const bool useInactiveButtonStyle, const QColor darkSymbolColor, const QColor lightSymbolColor ) const
    {
        QColor col;

        if ( useActiveButtonStyle || ( !inactiveWindow && !useInactiveButtonStyle ) )
            col = darkSymbolColor;
        else
        {
            auto d = qobject_cast<Decoration*>( decoration() );
            QColor titleBarColor ( d->titleBarColor() );

            uint r = qRed(titleBarColor.rgb());
            uint g = qGreen(titleBarColor.rgb());
            uint b = qBlue(titleBarColor.rgb());

            // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
            // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
            // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
            qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
            if ( colorConditional > 186 || g > 186 )
                col = darkSymbolColor;
            else
                col = lightSymbolColor;
        }
        return col;
    }

    bool Button::hovered() const
    {
      auto d = qobject_cast<Decoration*>( decoration() );
      return isHovered() || ( d->buttonHovered() && d->internalSettings()->unisonHovering() );
    }

    void Button::reconfigure()
    {

        // animation
        auto d = qobject_cast<Decoration*>(decoration());
        if( d )  m_animation->setDuration( d->internalSettings()->animationsDuration() );

    }

    void Button::updateAnimationState( bool hovered )
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if( !d || !d->internalSettings()->animationsEnabled() ) return;

        QAbstractAnimation::Direction dir = hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        if( m_animation->state() == QAbstractAnimation::Running && m_animation->direction() != dir )
            m_animation->stop();
        m_animation->setDirection( dir );
        if( m_animation->state() != QAbstractAnimation::Running ) m_animation->start();

    }

} // namespace
