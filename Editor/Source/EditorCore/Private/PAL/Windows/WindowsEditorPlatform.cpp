#include "EditorCore.h"

#include <Windows.h>

#include <ShObjIdl_core.h>

#include <locale>
#include <codecvt>
#include <string>

namespace CE::Editor
{
	static std::wstring ToWString(const std::string& stringToConvert)
	{
		std::wstring wideString = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(stringToConvert);
		return wideString;
	}

	static std::wstring ToWString(const String& string)
	{
		std::string stringToConvert = string.ToStdString();
		std::wstring wideString = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(stringToConvert);

		return wideString;
	}

	static std::string ToString(const std::wstring& wideString)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX{};

		return converterX.to_bytes(wideString);
	}

	IO::Path WindowsEditorPlatform::ShowSelectDirectoryDialog(const IO::Path& defaultPath)
	{
		IO::Path result{};

		IFileDialog* pfd = nullptr;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
			{
				pfd->SetOptions(FOS_PICKFOLDERS);
			}

			if (SUCCEEDED(pfd->Show(NULL)))
			{
				IShellItem* psi;
				if (SUCCEEDED(pfd->GetResult(&psi)))
				{
					LPWSTR pathString{};
					if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pathString)))
					{
						std::wstring wideString = std::wstring(pathString);
						result = ToString(wideString);
					}
					psi->Release();
				}
			}
			pfd->Release();
		}

		return result;
	}

	IO::Path WindowsEditorPlatform::ShowFileSelectionDialog(const IO::Path& defaultPath, const Array<FileType>& inFileTypes)
	{
		IO::Path result{};
		if (inFileTypes.IsEmpty())
			return result;

		IFileDialog* pfd = nullptr;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		{
			List<COMDLG_FILTERSPEC> fileTypes{};
			List<std::wstring> descStore{};
			List<std::wstring> extStore{};

			for (const auto& inFileType : inFileTypes)
			{
				descStore.Add(ToWString(inFileType.desc));
				std::string ext = "";
				for (int i = 0; i < inFileType.extensions.GetSize(); i++)
				{
					if (inFileType.extensions[i].StartsWith("*"))
						ext += inFileType.extensions[i].ToStdString();
					else
						ext += "*" + inFileType.extensions[i].ToStdString();

					if (i < inFileType.extensions.GetSize() - 1) // Not last element
						ext += ";";
				}
				extStore.Add(ToWString(ext));
			}

			for (int i = 0; i < descStore.GetSize(); i++)
			{
				fileTypes.Add({ descStore[i].c_str(), extStore[i].c_str() });
			}
			
			if (SUCCEEDED(pfd->SetFileTypes(fileTypes.GetSize(), fileTypes.GetData())))
			{
				if (SUCCEEDED(pfd->Show(NULL)))
				{
					IShellItem* psi;
					if (SUCCEEDED(pfd->GetResult(&psi)))
					{
						LPWSTR pathString{};
						if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pathString)))
						{
							std::wstring wideString = std::wstring(pathString);
							result = ToString(wideString);

							if (pathString)
								CoTaskMemFree(pathString);
						}
						psi->Release();
					}
				}
			}

			pfd->Release();
		}

		return result;
	}

	Array<IO::Path> WindowsEditorPlatform::ShowMultiFileSelectionDialog(const IO::Path& defaultPath,
		const Array<FileType>& inFileTypes)
	{
		Array<IO::Path> result{};
		if (inFileTypes.IsEmpty())
			return result;

		IFileOpenDialog* pfd = nullptr;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		{
			DWORD dwFlags;
			if (SUCCEEDED(pfd->GetOptions(&dwFlags)))
				pfd->SetOptions(dwFlags | FOS_ALLOWMULTISELECT);

			List<COMDLG_FILTERSPEC> fileTypes{};
			List<std::wstring> descStore{};
			List<std::wstring> extStore{};

			for (const auto& inFileType : inFileTypes)
			{
				descStore.Add(ToWString(inFileType.desc));
				std::string ext = "";
				for (int i = 0; i < inFileType.extensions.GetSize(); i++)
				{
					if (inFileType.extensions[i].StartsWith("*"))
						ext += inFileType.extensions[i].ToStdString();
					else
						ext += "*" + inFileType.extensions[i].ToStdString();

					if (i < inFileType.extensions.GetSize() - 1) // Not last element
						ext += ";";
				}
				extStore.Add(ToWString(ext));
			}

			for (int i = 0; i < descStore.GetSize(); i++)
			{
				fileTypes.Add({ descStore[i].c_str(), extStore[i].c_str() });
			}

			if (SUCCEEDED(pfd->SetFileTypes(fileTypes.GetSize(), fileTypes.GetData())))
			{
				if (SUCCEEDED(pfd->Show(NULL)))
				{
					IShellItemArray* psia = nullptr;
					if (SUCCEEDED(pfd->GetResults(&psia)))
					{
						DWORD count = 0;
						psia->GetCount(&count);
						for (DWORD i = 0; i < count; ++i)
						{
							IShellItem* psi = nullptr;
							if (SUCCEEDED(psia->GetItemAt(i, &psi)))
							{
								LPWSTR pathString{};
								if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pathString)))
								{
									result.Add(ToString(std::wstring(pathString)));
									CoTaskMemFree(pathString);
								}
								psi->Release();
							}
						}
						psia->Release();
					}
				}
			}

			pfd->Release();
		}

		return result;
	}

	IO::Path WindowsEditorPlatform::GetEditorExecutablePath()
	{
		return PlatformDirectories::GetLaunchDir() / GetEditorExecutableName();
	}

	Color WindowsEditorPlatform::GetScreenPixel(Vec2i screenPos)
	{
		HDC dc = GetDC(nullptr);

		COLORREF color = GetPixel(dc, screenPos.x, screenPos.y);

		Color retVal = Color::RGBA(GetRValue(color), GetGValue(color), GetBValue(color));

		ReleaseDC(nullptr, dc);

		return retVal;
	}

	bool WindowsEditorPlatform::OpenPathInFileExplorer(const IO::Path& path)
	{
		if (!path.Exists())
			return false;

		if (path.IsDirectory())
		{
			std::wstring widePath = ToWString(path.GetString().Replace({ '/' }, '\\'));
			ShellExecuteW(nullptr, L"open", widePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		}
		else
		{
			std::wstring widePath = ToWString(path.GetString().Replace({ '/' }, '\\'));
			std::wstring param = L"/select,\"" + widePath + L"\"";
			ShellExecuteW(nullptr, L"open", L"explorer.exe", param.c_str(), nullptr, SW_SHOWNORMAL);
		}

		return true;
	}

} // namespace CE::Editor
