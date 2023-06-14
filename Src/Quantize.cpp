// Quantize.cpp
//
// Dialog for quantizing (reducing) the colours used by an image.
//
// Copyright (c) 2023 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "imgui.h"
#include "Quantize.h"
#include "Image.h"
#include "TacentView.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;


void Viewer::DoQuantizeInterface(int& method, int& spatialFilterSize, float& spatialDitherLevel, int& neuSampleFactor, float itemWidth)
{
	if (itemWidth > 0.0f)
		ImGui::SetNextItemWidth(itemWidth);

	const char* methodItems[] = { "Fixed Palette", "Spatial Scolorq", "Neu Quant", "Wu Bipartition" };
	ImGui::Combo("Quantize", &method , methodItems, tNumElements(methodItems));
	ImGui::SameLine();
	ShowHelpMark
	(
		"The colour quantization method is used to create a high-quality palette,\n\n"

		"Fixed Palette: Lowest quality since it doesn't inspect the image pixels. It\n"
		"is useful for 1-bit palettes as it guarantees black and white for this size.\n\n"

		"Spatial Scolorq: High quality but slow. Good for 5-bit palettes and smaller.\n"
		"This is the only quantization method supporting dither.\n\n"

		"Neu Quant: Defacto high-quality quantizer. Neural-network based.\n"
		"Good for 6-bit palettes and larger.\n\n"

		"Wu Bipartition: The default quantizer. Fast and looks very good at 5-bit and up."
	);

	if (method == int(tQuantize::Method::Spatial))
	{
		if (itemWidth > 0.0f)
			ImGui::SetNextItemWidth(itemWidth);
		const char* filterSizeItems[] = { "Low", "Med", "High" };
		ImGui::Combo("Quantize Filter Size", &spatialFilterSize , filterSizeItems, tNumElements(filterSizeItems));
		ImGui::SameLine();
		ShowHelpMark("Filter size for the scolorq/spatial quantizer. Low -> 1, Med -> 3, High -> 5");

		if (itemWidth > 0.0f)
			ImGui::SetNextItemWidth(itemWidth);
 		ImGui::SliderFloat("Quantize Dither", &spatialDitherLevel, 0.0f, 2.0f, "%.1f");
		ImGui::SameLine();
		ShowHelpMark
		(
			"Dither level for the scolorq/spatial quantizer. 0 means auto-determine a good value for\n"
			"the current image based on its dimensions. Greater than zero means manually set the\n"
			"amount. A dither value of 0.1 results in no dithering. 2.0 results in significant dithering."
		);
	}

	if (method == int(tQuantize::Method::Neu))
	{
		if (itemWidth > 0.0f)
			ImGui::SetNextItemWidth(itemWidth);
		ImGui::SliderInt("Quantize Factor", &neuSampleFactor, 1, 10, "%d");
		ImGui::SameLine();
		ShowHelpMark
		(
			"Sample factor for the Neu quantizer. 1 is highest quality and slowest. All pixels are visited\n"
			"during the learning step. 10 means only 1/10th of the image is considered during learning."
		);
	}
}


//
// Below are the top-level modals.
//


void Viewer::DoQuantizeModal(bool quantizeImagePressed)
{
	if (quantizeImagePressed)
		ImGui::OpenPopup("Quantize");

	bool isOpenQuantizeImage = true;
	ImGui::SetNextWindowSize(tVector2(356.0f, 0.0f));
	if (!ImGui::BeginPopupModal("Quantize", &isOpenQuantizeImage, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	float buttonWidth;
	switch (Config::Current->GetUISize())
	{
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:
			buttonWidth		= 78.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Medium:
			buttonWidth		= 86.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Large:
			buttonWidth		= 94.0f;
			break;
	}

	tAssert(CurrImage);
	static int method = int(tImage::tQuantize::Method::Wu);
	static int spatialFilterSize = 1;
	static float spatialDitherLevel = 0.0f;
	static int neuSampleFactor = 1;
	const float itemWidth = 180.0f;
	DoQuantizeInterface(method, spatialFilterSize, spatialDitherLevel, neuSampleFactor, itemWidth);

	static int numColours = 256;
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Num Colours", &numColours);
	ImGui::SameLine();
	ShowHelpMark
	(
		"The number of colours you are quantizing to. Must be between 2 and 256."
	);
	tMath::tiClamp(numColours, 2, 256);

	static bool checkExact = true;
	ImGui::Checkbox("Check Exact", &checkExact);
	ImGui::SameLine();
	ShowHelpMark
	(
		"If Check-Exact is true pressing Quantize will inspect all image pixels in case there are\n"
		"fewer (or equal) colours than the number of colours requested. If this is true then the\n"
		"image is exactly representable already and the quantize is not needed. If false the\n"
		"quantize will proceed either way, and the chosen quantize method may adjust the colours."
	);

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (Viewer::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		method = int(tImage::tQuantize::Method::Wu);
		spatialFilterSize = 1;
		spatialDitherLevel = 0.0f;
		neuSampleFactor = 1;
		numColours = 256;
		checkExact = true;
	}

	ImGui::SameLine();
	if (Viewer::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - buttonWidth);
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("Quantize##Button", tVector2(buttonWidth, 0.0f)))
	{
		CurrImage->Unbind();
		switch (tImage::tQuantize::Method(method))
		{
			case tImage::tQuantize::Method::Fixed:
				CurrImage->QuantizeFixed(numColours, checkExact);
				break;

			case tImage::tQuantize::Method::Spatial:
			{
				int filterSize135 = (spatialFilterSize * 2) + 1;
				CurrImage->QuantizeSpatial(numColours, checkExact, spatialDitherLevel, filterSize135);
				break;
			}

			case tImage::tQuantize::Method::Neu:
				CurrImage->QuantizeNeu(numColours, checkExact, neuSampleFactor);
				break;

			case tImage::tQuantize::Method::Wu:
				CurrImage->QuantizeWu(numColours, checkExact);
				break;
		}
		CurrImage->Bind();
		Viewer::SetWindowTitle();

		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}
