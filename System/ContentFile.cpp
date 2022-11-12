#include "ContentFile.h"
#include "AudioMan.h"
#include "PresetMan.h"
#include "ConsoleMan.h"

#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "boost/functional/hash.hpp"

namespace RTE {

	const std::string ContentFile::c_ClassName = "ContentFile";

	std::array<std::unordered_map<std::string, BITMAP *>, ContentFile::BitDepths::BitDepthCount> ContentFile::s_LoadedBitmaps;
	std::unordered_map<std::string, FMOD::Sound *> ContentFile::s_LoadedSamples;
	std::unordered_map<size_t, std::string> ContentFile::s_PathHashes;
	std::vector<GLTextureInfo> ContentFile::s_GLTextures;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GLTextureInfo::GLTextureInfo(GLuint textureID): m_TexturePtr(textureID) {}

	GLTextureInfo::~GLTextureInfo() {
		glDeleteTextures(1, &m_TexturePtr);
		m_TexturePtr = 0;
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::Clear() {
		m_DataPath.clear();
		m_DataPathExtension.clear();
		m_DataPathWithoutExtension.clear();
		m_FormattedReaderPosition.clear();
		m_DataPathAndReaderPosition.clear();
		m_DataModuleID = 0;
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const char *filePath) {
		SetDataPath(filePath);
		SetFormattedReaderPosition(GetFormattedReaderPosition());

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const ContentFile &reference) {
		m_DataPath = reference.m_DataPath;
		m_DataPathExtension = reference.m_DataPathExtension;
		m_DataPathWithoutExtension = reference.m_DataPathWithoutExtension;
		m_DataModuleID = reference.m_DataModuleID;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::FreeAllLoaded() {
		for (int depth = BitDepths::Eight; depth < BitDepths::BitDepthCount; ++depth) {
			for (const auto &[bitmapPath, bitmapPtr] : s_LoadedBitmaps[depth]) {
				destroy_bitmap(bitmapPtr);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "FilePath" || propName == "Path") {
			SetDataPath(reader.ReadPropValue());
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Save(Writer &writer) const {
		Serializable::Save(writer);

		if (!m_DataPath.empty()) { writer.NewPropertyWithValue("FilePath", m_DataPath); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::GetDataModuleID() const { return (m_DataModuleID < 0) ? g_PresetMan.GetModuleIDFromPath(m_DataPath) : m_DataModuleID; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::SetDataPath(const std::string &newDataPath) {
		m_DataPath = CorrectBackslashesInPath(newDataPath);
		m_DataPathExtension = std::filesystem::path(m_DataPath).extension().string();

		RTEAssert(!m_DataPathExtension.empty(), "Failed to find file extension when trying to find file with path and name:\n" + m_DataPath + "\n" + GetFormattedReaderPosition());

		m_DataPathWithoutExtension = m_DataPath.substr(0, m_DataPath.length() - m_DataPathExtension.length());
		s_PathHashes[GetHash()] = m_DataPath;
		m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::SetFormattedReaderPosition(const std::string &newPosition) {
		m_FormattedReaderPosition = newPosition;
		m_DataPathAndReaderPosition = m_DataPath + "\n" + newPosition;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	size_t ContentFile::GetHash() const { return boost::hash<std::string>()(m_DataPath); }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::GetAsBitmap(int conversionMode, bool storeBitmap, const std::string &dataPathToSpecificFrame) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		BITMAP *returnBitmap = nullptr;
		const int bitDepth = (conversionMode == COLORCONV_8_TO_32) ? BitDepths::ThirtyTwo : BitDepths::Eight;
		std::string dataPathToLoad = dataPathToSpecificFrame.empty() ? m_DataPath : dataPathToSpecificFrame;
		SetFormattedReaderPosition(GetFormattedReaderPosition());

		// Check if the file has already been read and loaded from the disk and, if so, use that data.
		std::unordered_map<std::string, BITMAP *>::iterator foundBitmap = s_LoadedBitmaps[bitDepth].find(dataPathToLoad);
		if (foundBitmap != s_LoadedBitmaps[bitDepth].end()) {
			returnBitmap = (*foundBitmap).second;
		} else {
			if (!System::PathExistsCaseSensitive(dataPathToLoad)) {
				const std::string dataPathWithoutExtension = dataPathToLoad.substr(0, dataPathToLoad.length() - m_DataPathExtension.length());
				const std::string altFileExtension = (m_DataPathExtension == ".png") ? ".bmp" : ".png";

				if (System::PathExistsCaseSensitive(dataPathWithoutExtension + altFileExtension)) {
					g_ConsoleMan.AddLoadWarningLogEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
					SetDataPath(m_DataPathWithoutExtension + altFileExtension);
					dataPathToLoad = dataPathWithoutExtension + altFileExtension;
				} else {
					RTEAbort("Failed to find image file with following path and name:\n\n" + dataPathToLoad + " or " + altFileExtension + "\n" + m_FormattedReaderPosition);
				}
			}
			returnBitmap = LoadAndReleaseBitmap(conversionMode, dataPathToLoad); // NOTE: This takes ownership of the bitmap file

			// Insert the bitmap into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			if (storeBitmap) { s_LoadedBitmaps[bitDepth].try_emplace(dataPathToLoad, returnBitmap); }
		}
		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::GetAsAnimation(std::vector<BITMAP *> &vectorToFill, int frameCount, int conversionMode) {
		if (m_DataPath.empty() || frameCount < 1) {
			return;
		}
		vectorToFill.reserve(frameCount);
		SetFormattedReaderPosition(GetFormattedReaderPosition());

		if (frameCount == 1) {
			// Check for 000 in the file name in case it is part of an animation but the FrameCount was set to 1. Do not warn about this because it's normal operation, but warn about incorrect extension.
			if (!System::PathExistsCaseSensitive(m_DataPath)) {
				const std::string altFileExtension = (m_DataPathExtension == ".png") ? ".bmp" : ".png";

				if (System::PathExistsCaseSensitive(m_DataPathWithoutExtension + "000" + m_DataPathExtension)) {
					SetDataPath(m_DataPathWithoutExtension + "000" + m_DataPathExtension);
				} else if (System::PathExistsCaseSensitive(m_DataPathWithoutExtension + "000" + altFileExtension)) {
					g_ConsoleMan.AddLoadWarningLogEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
					SetDataPath(m_DataPathWithoutExtension + "000" + altFileExtension);
				}
			}
			vectorToFill.emplace_back(GetAsBitmap(conversionMode));
		} else {
			char framePath[1024];
			for (int frameNum = 0; frameNum < frameCount; ++frameNum) {
				std::snprintf(framePath, sizeof(framePath), "%s%03i%s", m_DataPathWithoutExtension.c_str(), frameNum, m_DataPathExtension.c_str());
				vectorToFill.emplace_back(GetAsBitmap(conversionMode, true, framePath));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::LoadAndReleaseBitmap(int conversionMode, const std::string &dataPathToSpecificFrame) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		const std::string dataPathToLoad = dataPathToSpecificFrame.empty() ? m_DataPath : dataPathToSpecificFrame;
		SetFormattedReaderPosition(GetFormattedReaderPosition());

		BITMAP *returnBitmap = nullptr;

		PALETTE currentPalette;
		get_palette(currentPalette);

		set_color_conversion((conversionMode == 0) ? COLORCONV_MOST : conversionMode);
		returnBitmap = load_bitmap(dataPathToLoad.c_str(), currentPalette);
		RTEAssert(returnBitmap, "Failed to load image file with following path and name:\n\n" + m_DataPathAndReaderPosition + "\nThe file may be corrupt, incorrectly converted or saved with unsupported parameters.");

		if(bitmap_color_depth(returnBitmap) == 32) {
			GLuint glTexture;
			glGenTextures(1, &glTexture);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, glTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, returnBitmap->w, returnBitmap->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, returnBitmap->line[0]);

			s_GLTextures.emplace_back(glTexture);
			returnBitmap->extra = static_cast<void*>(&(s_GLTextures.back()));
		}

		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::GetAsSound(bool abortGameForInvalidSound, bool asyncLoading) {
		if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
			return nullptr;
		}
		FMOD::Sound *returnSample = nullptr;

		std::unordered_map<std::string, FMOD::Sound *>::iterator foundSound = s_LoadedSamples.find(m_DataPath);
		if (foundSound != s_LoadedSamples.end()) {
			returnSample = (*foundSound).second;
		} else {
			returnSample = LoadAndReleaseSound(abortGameForInvalidSound, asyncLoading); //NOTE: This takes ownership of the sample file

			// Insert the Sound object into the map, PASSING OVER OWNERSHIP OF THE LOADED FILE
			s_LoadedSamples.try_emplace(m_DataPath, returnSample);
		}
		return returnSample;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::LoadAndReleaseSound(bool abortGameForInvalidSound, bool asyncLoading) {
		if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
			return nullptr;
		}

		if (!System::PathExistsCaseSensitive(m_DataPath)) {
			bool foundAltExtension = false;
			for (const std::string &altFileExtension : c_SupportedAudioFormats) {
				if (System::PathExistsCaseSensitive(m_DataPathWithoutExtension + altFileExtension)) {
					g_ConsoleMan.AddLoadWarningLogEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
					SetDataPath(m_DataPathWithoutExtension + altFileExtension);
					foundAltExtension = true;
					break;
				}
			}
			if (!foundAltExtension) {
				std::string errorMessage = "Failed to find audio file with following path and name:\n\n" + m_DataPath + " or any alternative supported file type";
				RTEAssert(!abortGameForInvalidSound, errorMessage + "\n" + m_FormattedReaderPosition);
				g_ConsoleMan.PrintString(errorMessage + ". The file was not loaded!");
				return nullptr;
			}
		}
		if (std::filesystem::file_size(m_DataPath) == 0) {
			const std::string errorMessage = "Failed to create sound because the file was empty. The path and name were: ";
			RTEAssert(!abortGameForInvalidSound, errorMessage + "\n\n" + m_DataPathAndReaderPosition);
			g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
			return nullptr;
		}
		FMOD::Sound *returnSample = nullptr;

		FMOD_MODE fmodFlags = FMOD_CREATESAMPLE | FMOD_3D | (asyncLoading ? FMOD_NONBLOCKING : FMOD_DEFAULT);
		FMOD_RESULT result = g_AudioMan.GetAudioSystem()->createSound(m_DataPath.c_str(), fmodFlags, nullptr, &returnSample);

		if (result != FMOD_OK) {
			const std::string errorMessage = "Failed to create sound because of FMOD error:\n" + std::string(FMOD_ErrorString(result)) + "\nThe path and name were: ";
			RTEAssert(!abortGameForInvalidSound, errorMessage + "\n\n" + m_DataPathAndReaderPosition);
			g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
			return returnSample;
		}
		return returnSample;
	}
}
