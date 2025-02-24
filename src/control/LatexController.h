/*
 * Xournal++
 *
 * Controller for Latex stuff
 *
 * @author W Brenna
 * http://wbrenna.ca
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"
#include "model/Text.h"

#include <Path.h>
#include <XournalType.h>
#include "gui/dialog/LatexDialog.h"

#include <poppler.h>

#include <memory>

class Control;
class TexImage;
class Text;
class Document;
class XojPageView;
class Layer;

class LatexController
{
public:
	LatexController() = delete;
	LatexController(const LatexController& other) = delete;
	LatexController& operator=(const LatexController& other) = delete;
	LatexController(Control* control);
	virtual ~LatexController();

public:
	/**
	 * Open a LatexDialog, wait for the user to provide the LaTeX formula, and
	 * insert the rendered formula into the document if the supplied LaTeX is
	 * valid.
	 */
	void run();

private:
	/**
	 * Find the tex executable, return false if not found
	 */
	bool findTexExecutable();

	/**
	 * Find a selected tex element, and load it
	 */
	void findSelectedTexElement();

	/**
	 * If a previous image/text is selected, delete it
	 */
	void deleteOldImage();

	/**
	 * Run the LaTeX command asynchronously to generate a preview for the given
	 * LaTeX string. Note that this method can only be called when the preview
	 * is not updating.
	 *
	 * @return The PID of the spawned process, or nullptr if the .tex file could
	 * not be written or the command failed to start.
	 */
	std::unique_ptr<GPid> runCommandAsync(string texString);

	/**
	 * Asynchronously runs the LaTeX command and then updates the TeX image with
	 * the given LaTeX string. If the preview is already being updated, then
	 * this method will be a no-op.
	 */
	void triggerImageUpdate(string texString);

	/**
	 * Show the LaTex Editor dialog, returning the final formula input by the
	 * user. If the input was cancelled, the resulting string will be the same
	 * as the initial formula.
	 */
	string showTexEditDialog();

	/**
	 * Signal handler, updates the rendered image when the text in the editor
	 * changes.
	 */
	static void handleTexChanged(GtkTextBuffer* buffer, LatexController* self);

	/**
	 * Updates the display once the PDF file is generated.
	 *
	 * If the Latex text has changed since the last update, triggerPreviewUpdate
	 * will be called again.
	 */
	static void onPdfRenderComplete(GPid pid, gint returnCode, LatexController* self);

	void setUpdating(bool newValue);

	/**
	 * Convert the given PDF Document to a TexImage and set the formula to the
	 * given formula.
	 */
	std::unique_ptr<TexImage> convertDocumentToImage(PopplerDocument* doc, string formula);

	/**
	 * Load the preview PDF from disk and create a TexImage object.
	 */
	std::unique_ptr<TexImage> loadRendered(string renderedTex);

	/**
	 * Insert the generated preview TexImage into the current page.
	 */
	void insertTexImage();

private:
	XOJ_TYPE_ATTRIB;

	Control* control = NULL;

	/**
	 * LaTex editor dialog
	 */
	LatexDialog dlg;

	/**
	 * Tex binary full path
	 */
	Path pdflatexPath;

	/**
	 * The original TeX string when the dialog was opened, or the empty string
	 * if creating a new LaTeX element.
	 */
	string initialTex;

	/**
	 * The last TeX string shown in the preview.
	 */
	string lastPreviewedTex;

	/**
	 * Whether a preview is currently being generated.
	 */
	bool isUpdating = false;

	/**
	 * Whether the current TeX string is valid.
	 */
	bool isValidTex = false;

	/**
	 * X-Position
	 */
	int posx = 0;

	/**
	 * Y-Position
	 */
	int posy = 0;

	/**
	 * Image width
	 */
	int imgwidth = 0;

	/**
	 * Image height
	 */
	int imgheight = 0;

	/**
	 * Document
	 */
	Document* doc = NULL;

	/**
	 * Page View
	 */
	XojPageView* view = NULL;

	/**
	 * Selected Page
	 */
	PageRef page;

	/**
	 * Selected layer
	 */
	Layer* layer = NULL;

	/**
	 * The directory in which the LaTeX files will be generated. Note that this
	 * should be within a system temporary directory.
	 */
	Path texTmpDir;

	/**
	 * Previously existing TexImage
	 */
	TexImage* selectedTexImage = NULL;

	Text* selectedText = NULL;

	/**
	 * The controller owns the rendered preview in order to be able to delete it
	 * when a new render is created
	 */
	std::unique_ptr<TexImage> temporaryRender;
};
