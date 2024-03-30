/*
Weborf
Copyright (C) 2010  Salvo "LtWorf" Tomaselli

Weborf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

@author Salvo "LtWorf" Tomaselli <tiposchi@tiscali.it>
*/
#include "options.h"
#include <stdbool.h>

#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

typedef struct mimetype_t
{
    const char *name;
    const char **exts;
} mimetype_t;

const mimetype_t mimetype_map[] = {
    {
        "text/html",
        (const char *[]){
            ".html",
            ".htm",
            ".shtml",
            ".h5",
            NULL,
        },
    },
    {
        "text/css",
        (const char *[]){
            ".css",
            ".scss",
            NULL,
        },
    },
    {
        "text/xml",
        (const char *[]){
            ".xml",
            NULL,
        },
    },
    {
        "image/gif",
        (const char *[]){
            ".gif",
            NULL,
        },
    },
    {
        "image/jpeg",
        (const char *[]){
            ".jpeg",
            ".jpg",
            NULL,
        },
    },
    {
        "application/javascript",
        (const char *[]){
            ".js",
            ".ts",
            NULL,
        },
    },
    {
        "application/atom+xml",
        (const char *[]){
            ".atom",
            NULL,
        },
    },
    {
        "application/rss+xml",
        (const char *[]){
            ".rss",
            NULL,
        },
    },
    {
        "text/plain",
        (const char *[]){
            ".txt",
            ".log",
            ".pid",
            NULL,
        },
    },
    {
        "text/vnd.sun.j2me.app-descriptor",
        (const char *[]){
            ".jad",
            NULL,
        },
    },
    {
        "text/vnd.wap.wml",
        (const char *[]){
            ".wml",
            NULL,
        },
    },
    {
        "text/x-component",
        (const char *[]){
            ".htc",
            NULL,
        },
    },
    {
        "image/png",
        (const char *[]){
            ".png",
            NULL,
        },
    },
    {
        "image/svg+xml",
        (const char *[]){
            ".svg",
            ".svgz",
            NULL,
        },
    },
    {
        "image/tiff",
        (const char *[]){
            ".tif",
            ".tiff",
            NULL,
        },
    },
    {
        "image/vnd.wap.wbmp",
        (const char *[]){
            ".wbmp",
            NULL,
        },
    },
    {
        "image/webp",
        (const char *[]){
            ".webp",
            NULL,
        },
    },
    {
        "image/x-icon",
        (const char *[]){
            ".ico",
            NULL,
        },
    },
    {
        "image/x-ms-bmp",
        (const char *[]){
            ".bmp",
            NULL,
        },
    },
    {
        "font/woff",
        (const char *[]){
            ".woff",
            NULL,
        },
    },
    {
        "font/woff2",
        (const char *[]){
            ".woff2",
            NULL,
        },
    },
    {
        "application/java-archive",
        (const char *[]){
            ".jar",
            ".war",
            ".ear",
            NULL,
        },
    },
    {
        "application/json",
        (const char *[]){
            ".json",
            NULL,
        },
    },
    {
        "application/mac-binhex40",
        (const char *[]){
            ".hqx",
            NULL,
        },
    },
    {
        "application/msword",
        (const char *[]){
            ".doc",
            NULL,
        },
    },
    {
        "application/pdf",
        (const char *[]){
            ".pdf",
            NULL,
        },
    },
    {
        "application/postscript",
        (const char *[]){
            ".ps",
            ".eps",
            ".ai",
            NULL,
        },
    },
    {
        "application/rtf",
        (const char *[]){
            ".rtf",
            NULL,
        },
    },
    {
        "application/vnd.apple.mpegurl",
        (const char *[]){
            ".m3u8",
            NULL,
        },
    },
    {
        "application/vnd.google-earth.kml+xml",
        (const char *[]){
            ".kml",
            NULL,
        },
    },
    {
        "application/vnd.google-earth.kmz",
        (const char *[]){
            ".kmz",
            NULL,
        },
    },
    {
        "application/vnd.ms-excel",
        (const char *[]){
            ".xls",
            NULL,
        },
    },
    {
        "application/vnd.ms-fontobject",
        (const char *[]){
            ".eot",
            NULL,
        },
    },
    {
        "application/vnd.ms-powerpoint",
        (const char *[]){
            ".ppt",
            NULL,
        },
    },
    {
        "application/vnd.oasis.opendocument.graphics",
        (const char *[]){
            ".odg",
            NULL,
        },
    },
    {
        "application/vnd.oasis.opendocument.presentation",
        (const char *[]){
            ".odp",
            NULL,
        },
    },
    {
        "application/vnd.oasis.opendocument.spreadsheet",
        (const char *[]){
            ".ods",
            NULL,
        },
    },
    {
        "application/vnd.oasis.opendocument.text",
        (const char *[]){
            ".odt",
            NULL,
        },
    },
    {
        "application/vnd.openxmlformats-officedocument.presentationml.presentation",
        (const char *[]){
            ".pptx",
            NULL,
        },
    },
    {
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
        (const char *[]){
            ".xlsx",
            NULL,
        },
    },
    {
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        (const char *[]){
            ".docx",
            NULL,
        },
    },
    {
        "application/vnd.wap.wmlc",
        (const char *[]){
            ".wmlc",
            NULL,
        },
    },
    {
        "application/x-7z-compressed",
        (const char *[]){
            ".7z",
            NULL,
        },
    },
    {
        "application/x-cocoa",
        (const char *[]){
            ".cco",
            NULL,
        },
    },
    {
        "application/x-java-archive-diff",
        (const char *[]){
            ".jardiff",
            NULL,
        },
    },
    {
        "application/x-java-jnlp-file",
        (const char *[]){
            ".jnlp",
            NULL,
        },
    },
    {
        "application/x-makeself",
        (const char *[]){
            ".run",
            NULL,
        },
    },
    {
        "application/x-perl",
        (const char *[]){
            ".pl",
            ".pm",
            NULL,
        },
    },
    {
        "application/x-rar-compressed",
        (const char *[]){
            ".rar",
            NULL,
        },
    },
    {
        "application/x-redhat-package-manager",
        (const char *[]){
            ".rpm",
            NULL,
        },
    },
    {
        "application/x-sea",
        (const char *[]){
            ".sea",
            NULL,
        },
    },
    {
        "application/x-shockwave-flash",
        (const char *[]){
            ".swf",
            NULL,
        },
    },
    {
        "application/x-x509-ca-cert",
        (const char *[]){
            ".der",
            ".pem",
            ".crt",
            NULL,
        },
    },
    {
        "application/x-xpinstall",
        (const char *[]){
            ".xpi",
            NULL,
        },
    },
    {
        "application/xhtml+xml",
        (const char *[]){
            ".xhtml",
            NULL,
        },
    },
    {
        "application/xspf+xml",
        (const char *[]){
            ".xspf",
            NULL,
        },
    },
    {
        "application/zip",
        (const char *[]){
            ".zip",
            NULL,
        },
    },
    {
        "audio/midi",
        (const char *[]){
            ".mid",
            ".midi",
            ".kar",
            NULL,
        },
    },
    {
        "audio/mpeg",
        (const char *[]){
            ".mp3",
            NULL,
        },
    },
    {
        "audio/ogg",
        (const char *[]){
            ".ogg",
            ".opus",
            NULL,
        },
    },
    {
        "audio/x-m4a",
        (const char *[]){
            ".m4a",
            ".aac",
            NULL,
        },
    },
    {
        "audio/x-realaudio",
        (const char *[]){
            ".ra",
            NULL,
        },
    },
    {
        "video/3gpp",
        (const char *[]){
            ".3gpp",
            ".3gp",
            NULL,
        },
    },
    {
        "video/mp2t",
        (const char *[]){
            ".ts",
            ".m2ts",
            NULL,
        },
    },
    {
        "video/mp4",
        (const char *[]){
            ".mp4",
            NULL,
        },
    },
    {
        "video/mpeg",
        (const char *[]){
            ".mpeg",
            ".mpg",
            NULL,
        },
    },
    {
        "video/quicktime",
        (const char *[]){
            ".mov",
            NULL,
        },
    },
    {
        "video/webm",
        (const char *[]){
            ".webm",
            ".mkv",
            NULL,
        },
    },
    {
        "video/x-flv",
        (const char *[]){
            ".flv",
            NULL,
        },
    },
    {
        "video/x-m4v",
        (const char *[]){
            ".m4v",
            NULL,
        },
    },
    {
        "video/x-msvideo",
        (const char *[]){
            ".avi",
            NULL,
        },
    },
    {
        0,
    }};

char *strlowr(char *str)
{
    char *orign = str;
    for (; *str != '\0'; str++)
    {
        *str = tolower(*str);
    }
    return orign;
}

/**
returns mimetype of an filextension
*/
const char *get_mime(const char *fname)
{

    if (!fname)
        return MIME_DEFAULT;

    const mimetype_t *itr = mimetype_map;
    char *needle = strrchr(fname, '.');
    *needle = *strlowr(needle);

    size_t len = strlen(needle);

    while (itr->name)
    {
        const char **ext = itr->exts;
        while (*ext)
        {
            const bool match =
                strncmp(*ext, needle, len) == 0 && (*ext)[len] == 0;
            if (match)
            {
                return itr->name;
            }
            ++ext;
        }
        ++itr;
    }

    return MIME_DEFAULT;
}