#pragma once

namespace CE::Editor
{

	class EDITORCORE_API WindowsEditorPlatform : public EditorPlatformBase
	{
		CE_STATIC_CLASS(WindowsEditorPlatform)
	public:
		
		static IO::Path ShowSelectDirectoryDialog(const IO::Path& defaultPath);

		static IO::Path ShowFileSelectionDialog(const IO::Path& defaultPath, const Array<FileType>& fileTypes);

		static Array<IO::Path> ShowMultiFileSelectionDialog(const IO::Path& defaultPath, const Array<FileType>& fileTypes);

		static IO::Path GetEditorExecutablePath();

		static Color GetScreenPixel(Vec2i screenPos);

		static bool OpenPathInFileExplorer(const IO::Path& path);

	};

	typedef WindowsEditorPlatform EditorPlatform;
    
} // namespace CE::Editor
