#include <map>
#include <iostream>

std::map<std::string, std::string> mimeTypes()
{
    std::map<std::string, std::string> mime_map;

	mime_map["txt"]		= "text/plain";
	mime_map["html"]	= "text/html";
	mime_map["css"]		= "text/css";
	mime_map["js"]		= "text/javascript";

	mime_map["json"]	= "application/json";
	mime_map["jsonld"]	= "application/ld+json";
	mime_map["xml"]		= "application/xml";
	mime_map["pdf"]		= "application/pdf";
	mime_map["doc"]		= "application/msword";
	mime_map["docx"]	= "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mime_map["ppt"]		= "application/vnd.ms-powerpoint";
	mime_map["pptx"]	= "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mime_map["odt"]		= "application/vnd.oasis.opendocument.text";
	mime_map["xls"]		= "application/vnd.ms-excel";
	mime_map["xlsx"]	= "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mime_map["odp"]		= "application/vnd.oasis.opendocument.presentation";
	mime_map["ods"]		= "application/vnd.oasis.opendocument.spreadsheet";
	mime_map["bz"]		= "application/x-bzip";
	mime_map["bz2"]		= "application/x-bzip2";
	mime_map["gz"]		= "application/gzip";
	mime_map["zip"]		= "application/zip";
	mime_map["7z"]		= "application/x-7z-compressed";
	mime_map["tar"]		= "application/x-tar";

	mime_map["jpeg"]	= "image/jpeg";
	mime_map["jpg"]		= "image/jpeg";
	mime_map["png"]		= "image/png";
	mime_map["apng"]	= "image/apng";
	mime_map["avif"]	= "image/avif";
	mime_map["gif"]		= "image/gif";
	mime_map["svg"]		= "image/svg+xml";
	mime_map["webp"]	= "image/webp";
	mime_map["bmp"]		= "image/bmp";
	mime_map["ico"]		= "image/x-icon";
	mime_map["tif"]		= "image/tiff";
	mime_map["tiff"]	= "image/tiff";

	mime_map["mp3"]		= "audio/mpeg";
	mime_map["aac"]		= "audio/aac";
	mime_map["wav"]		= "audio/wave";
	mime_map["flac"]	= "audio/flac";
	mime_map["mpeg"]	= "audio/mpeg";
	mime_map["webm"]	= "video/webm";
	mime_map["mp4"]		= "video/mp4";
	mime_map["avi"]		= "video/x-msvideo";
	mime_map["3gp"]		= "video/3gpp";

    return mime_map;
}


std::map<std::string, std::string> post_type()
{
    std::map<std::string, std::string> mime_map;

    // Original mapping
    mime_map["text/plain"] = "txt";
    mime_map["text/html"] = "html";
    mime_map["text/css"] = "css";
    mime_map["text/javascript"] = "js";
    mime_map["text/xml"] = "xml";

    mime_map["application/json"] = "json";
    mime_map["application/x-www-form-urlencoded"] = "txt";
    mime_map["multipart/form-data"] = "bin";
    mime_map["application/ld+json"] = "jsonld";
    mime_map["application/xml"] = "xml";
    mime_map["application/pdf"] = "pdf";
    mime_map["application/msword"] = "doc";
    mime_map["application/vnd.openxmlformats-officedocument.wordprocessingml.document"] = "docx";
    mime_map["application/vnd.ms-powerpoint"] = "ppt";
    mime_map["application/vnd.openxmlformats-officedocument.presentationml.presentation"] = "pptx";
    mime_map["application/vnd.oasis.opendocument.text"] = "odt";
    mime_map["application/vnd.ms-excel"] = "xls";
    mime_map["application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"] = "xlsx";
    mime_map["application/vnd.oasis.opendocument.presentation"] = "odp";
    mime_map["application/vnd.oasis.opendocument.spreadsheet"] = "ods";
    mime_map["application/x-bzip"] = "bz";
    mime_map["application/x-bzip2"] = "bz2";
    mime_map["application/gzip"] = "gz";
    mime_map["application/zip"] = "zip";
    mime_map["application/x-7z-compressed"] = "7z";
    mime_map["application/x-tar"] = "tar";

    mime_map["image/jpeg"] = "jpeg";
    mime_map["image/png"] = "png";
    mime_map["image/apng"] = "apng";
    mime_map["image/avif"] = "avif";
    mime_map["image/gif"] = "gif";
    mime_map["image/svg+xml"] = "svg";
    mime_map["image/webp"] = "webp";
    mime_map["image/bmp"] = "bmp";
    mime_map["image/x-icon"] = "ico";
    mime_map["image/tiff"] = "tif";

    mime_map["audio/mpeg"] = "mp3";
    mime_map["audio/aac"] = "aac";
    mime_map["audio/wave"] = "wav";
    mime_map["audio/flac"] = "flac";

    mime_map["video/webm"] = "webm";
    mime_map["video/mp4"] = "mp4";
    mime_map["video/x-msvideo"] = "avi";
    mime_map["video/3gpp"] = "3gp";

    // Swap keys with values

    return mime_map;
}
