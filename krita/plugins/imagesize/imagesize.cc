/*
 * imagesize.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_scale_visitor.h>

#include "imagesize.h"
#include "dlg_imagesize.h"

typedef KGenericFactory<ImageSize> ImageSizeFactory;
K_EXPORT_COMPONENT_FACTORY( kritaimagesize, ImageSizeFactory( "krita" ) )

ImageSize::ImageSize(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
	setInstance(ImageSizeFactory::instance());

 	kdDebug() << "Imagesize plugin. Class: " 
 		  << className() 
 		  << ", Parent: " 
 		  << parent -> className()
 		  << "\n";

	(void) new KAction(i18n("&Image Size..."), 0, 0, this, SLOT(slotImageSize()), actionCollection(), "imagesize");
	(void) new KAction(i18n("&Layer Size..."), 0, 0, this, SLOT(slotLayerSize()), actionCollection(), "layersize");

	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = (KisView*) parent;
		// Selection manager takes ownership?
		KAction * a = new KAction(i18n("&Layer Size..."), 0, 0, this, SLOT(slotLayerSize()), actionCollection(), "selectionScale");
		m_view -> selectionManager() -> addSelectionAction(a);
	}
}

ImageSize::~ImageSize()
{
	m_view = 0;
}

void ImageSize::slotImageSize()
{
	KisImageSP image = m_view -> currentImg();

	if (!image) return;

	DlgImageSize * dlgImageSize = new DlgImageSize(m_view, "ImageSize");
	dlgImageSize -> setCaption(i18n("Image Size"));

	KisConfig cfg;

	dlgImageSize -> setWidth(image -> width());
	dlgImageSize -> setHeight(image -> height());
	dlgImageSize -> setMaximumWidth(cfg.maxImgWidth());
	dlgImageSize -> setMaximumHeight(cfg.maxImgHeight());

	if (dlgImageSize -> exec() == QDialog::Accepted) {
		m_view -> updateCanvas(); // XXX: Check whether this
					  // is necessary after
					  // Adrian's changes 
		Q_INT32 w = dlgImageSize -> width();
		Q_INT32 h = dlgImageSize -> height();
		
		if (dlgImageSize -> scale()) {

			Q_INT32 f = dlgImageSize -> filterType();
			m_view -> scaleCurrentImage((double)w / ((double)(image -> width())), 
						    (double)h / ((double)(image -> height())),
						    (enumFilterType)f);
		}
		else {
			m_view -> resizeCurrentImage(w, h);
		}
		
	}
	delete dlgImageSize;
}

void ImageSize::slotLayerSize()
{
	KisImageSP image = m_view -> currentImg();

	if (!image) return;

	DlgImageSize * dlgImageSize = new DlgImageSize(m_view, "LayerSize");
	dlgImageSize -> setCaption(i18n("Layer Size"));

	KisConfig cfg;

	dlgImageSize -> setWidth(image -> width());
	dlgImageSize -> setHeight(image -> height());
	dlgImageSize -> setMaximumWidth(cfg.maxImgWidth());
	dlgImageSize -> setMaximumHeight(cfg.maxImgHeight());

	if (dlgImageSize -> exec() == QDialog::Accepted) {
		Q_INT32 w = dlgImageSize -> width();
		Q_INT32 h = dlgImageSize -> height();
		Q_INT32 f = dlgImageSize -> filterType();
		if (dlgImageSize -> scale()) {
			m_view -> scaleLayer((double)w / ((double)(image -> width())), 
					     (double)h / ((double)(image -> height())),
					     (enumFilterType)f);
		}
		else {
			// AUTOLAYER should we crop layers ?
		}
		
	}
	delete dlgImageSize;
}

void ImageSize::slotSelectionScale()
{
	// XXX: figure out a way to add selection actions to the selection
	// manager to enable/disable

	KisImageSP image = m_view -> currentImg();

	if (!image) return;

	KisLayerSP layer = image -> activeLayer();

	if (!layer) return;

	if (!layer -> hasSelection()) return;
	

	DlgImageSize * dlgImageSize = new DlgImageSize(m_view, "SelectionScale");
	dlgImageSize -> setCaption(i18n("Scale Selection"));

	KisConfig cfg;

	dlgImageSize -> setWidth(image -> width());
	dlgImageSize -> setHeight(image -> height());
	dlgImageSize -> setMaximumWidth(cfg.maxImgWidth());
	dlgImageSize -> setMaximumHeight(cfg.maxImgHeight());

	dlgImageSize -> hideScaleBox();

	if (dlgImageSize -> exec() == QDialog::Accepted) {
		Q_INT32 w = dlgImageSize -> width();
		Q_INT32 h = dlgImageSize -> height();
		Q_INT32 f = dlgImageSize -> filterType();

		m_view -> scaleLayer((double)w / ((double)(image -> width())), 
				     (double)h / ((double)(image -> height())),
				     (enumFilterType)f);
		
	}
	delete dlgImageSize;
}


#include "imagesize.moc"
