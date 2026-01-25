
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Cocoa/Cocoa.h>

#include "EditorCore.h"

#include <string>

namespace CE::Editor
{

    IO::Path MacEditorPlatform::GetEditorExecutablePath()
    {
        return PlatformDirectories::GetLaunchDir() / GetEditorExecutableName();
    }

    IO::Path MacEditorPlatform::ShowSelectDirectoryDialog(const IO::Path& defaultPath)
    {
        IO::Path result{};
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseDirectories:YES];
        [panel setCanChooseFiles:NO];
        [panel setCanCreateDirectories:YES];
        [panel setAllowsMultipleSelection:NO];
        [panel setTitle:@"Select folder"];
        
        NSInteger ret = [panel runModal];
        
        if (ret == NSModalResponseOK)
        {
            NSURL* url = [panel URL];
            result = std::string([[url path] UTF8String]);
        }
        
        return result;
    }

    IO::Path MacEditorPlatform::ShowFileSelectionDialog(const IO::Path& defaultPath, const Array<FileType>& inFileTypes)
    {
        IO::Path result{};
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseDirectories:NO];
        [panel setCanChooseFiles:YES];
        [panel setCanCreateDirectories:YES];
        [panel setAllowsMultipleSelection:NO];
        [panel setTitle:@"Select file"];
        
        NSMutableArray<NSString*>* fileTypes = [[NSMutableArray<NSString*> alloc] init];
        
        for (const auto& fileType : inFileTypes)
        {
            for (const auto& extension : fileType.extensions)
            {
                String string = extension;
                if (string.StartsWith("*."))
                {
                    string = string.GetSubstring(2);
                }
                else if (string.StartsWith("."))
                {
                    string = string.GetSubstring(1);
                }
                [fileTypes addObject:[[NSString alloc] initWithCString:string.GetCString()]];
            }
        }
        
        [panel setAllowedFileTypes:fileTypes];

        NSInteger ret = [panel runModal];

        if (ret == NSModalResponseOK)
        {
            NSURL* url = [panel URL];
            result = std::string([[url path] UTF8String]);
        }

        return result;
    }

    Array<IO::Path> MacEditorPlatform::ShowMultiFileSelectionDialog(const IO::Path& defaultPath, const Array<FileType>& inFileTypes)
    {
        Array<IO::Path> results{};
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseDirectories:NO];
        [panel setCanChooseFiles:YES];
        [panel setCanCreateDirectories:YES];
        [panel setAllowsMultipleSelection:YES];
        [panel setTitle:@"Select file"];

        NSMutableArray<NSString*>* fileTypes = [[NSMutableArray<NSString*> alloc] init];

        for (const auto& fileType : inFileTypes)
        {
            for (const auto& extension : fileType.extensions)
            {
                String string = extension;
                if (string.StartsWith("*."))
                {
                    string = string.GetSubstring(2);
                }
                else if (string.StartsWith("."))
                {
                    string = string.GetSubstring(1);
                }
                [fileTypes addObject:[[NSString alloc] initWithCString:string.GetCString()]];
            }
        }

        [panel setAllowedFileTypes:fileTypes];

        NSInteger ret = [panel runModal];

        if (ret == NSModalResponseOK)
        {
            NSArray<NSURL*>* urls = [panel URLs];
            for (NSURL* url in urls)
            {
                String path = [[url path] UTF8String];
                results.Add(path);
            }
        }

        return results;
    }

    bool MacEditorPlatform::OpenPathInFileExplorer(const IO::Path& path)
    {
        if (!path.Exists())
            return false;

        std::string pathStr = path.GetString();

        // Escape quotes in path
        std::string escapedPath = "\"";
        escapedPath += pathStr;
        escapedPath += "\"";

        std::string command;
        if (path.IsDirectory())
        {
            command = "open " + escapedPath;
        }
        else
        {
            command = "open -R " + escapedPath;
        }

        int result = std::system(command.c_str());
        return result == 0;
    }


} // namespace CE::Editor
