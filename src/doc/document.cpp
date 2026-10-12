/*
 * wView5 - webview-like HTML library for PS5 homebrew.
 * Copyright (C) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "wview/document.hpp"

namespace wv
{
namespace
{

bool has_scheme(std::string_view ref)
{
    for (std::size_t i = 0; i < ref.size(); ++i)
    {
        char c = ref[i];
        if (c == ':')
        {
            return i > 0;
        }
        if (c == '/' || c == '?' || c == '#')
        {
            return false;
        }
    }
    return false;
}

std::string_view scheme_of(std::string_view url)
{
    for (std::size_t i = 0; i < url.size(); ++i)
    {
        if (url[i] == ':')
        {
            return url.substr(0, i);
        }
    }
    return {};
}

std::string_view authority_of(std::string_view url)
{
    std::size_t scheme = url.find("://");
    if (scheme == std::string_view::npos)
    {
        return {};
    }
    std::size_t start = scheme + 3;
    std::size_t end = url.find('/', start);
    if (end == std::string_view::npos)
    {
        end = url.size();
    }
    return url.substr(start, end - start);
}

std::string normalize_path(std::string_view path)
{
    std::vector<std::string> parts;
    std::size_t i = 0;
    bool absolute = !path.empty() && path[0] == '/';
    while (i < path.size())
    {
        while (i < path.size() && path[i] == '/')
        {
            ++i;
        }
        std::size_t j = i;
        while (j < path.size() && path[j] != '/')
        {
            ++j;
        }
        if (j > i)
        {
            std::string_view segment = path.substr(i, j - i);
            if (segment == ".")
            {
            }
            else if (segment == "..")
            {
                if (!parts.empty())
                {
                    parts.pop_back();
                }
            }
            else
            {
                parts.emplace_back(segment);
            }
        }
        i = j;
    }
    std::string out;
    if (absolute)
    {
        out.push_back('/');
    }
    for (std::size_t k = 0; k < parts.size(); ++k)
    {
        if (k > 0)
        {
            out.push_back('/');
        }
        out += parts[k];
    }
    if (out.empty())
    {
        out = absolute ? "/" : ".";
    }
    return out;
}

bool script_type_runs(std::string_view type)
{
    if (type.empty())
    {
        return true;
    }
    std::string lower;
    lower.reserve(type.size());
    for (char c : type)
    {
        lower.push_back((c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c);
    }
    return lower == "text/javascript" || lower == "application/javascript" ||
           lower == "text/ecmascript" || lower == "application/ecmascript";
}

bool stylesheet_media_runs(std::string_view media)
{
    if (media.empty())
    {
        return true;
    }
    std::string lower;
    lower.reserve(media.size());
    for (char c : media)
    {
        lower.push_back((c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c);
    }
    return lower.find("screen") != std::string::npos ||
           lower.find("all") != std::string::npos;
}

bool rel_is_stylesheet(std::string_view rel)
{
    std::string lower;
    lower.reserve(rel.size());
    for (char c : rel)
    {
        lower.push_back((c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c);
    }
    std::size_t i = 0;
    while (i < lower.size())
    {
        while (i < lower.size() && lower[i] == ' ')
        {
            ++i;
        }
        std::size_t j = i;
        while (j < lower.size() && lower[j] != ' ')
        {
            ++j;
        }
        if (j > i && lower.compare(i, j - i, "stylesheet") == 0)
        {
            return true;
        }
        i = j;
    }
    return false;
}

std::string element_text(const Document& doc, int node)
{
    std::string out;
    for (int c = doc.node(node).first_child; c != k_no_node;
         c = doc.node(c).next_sibling)
    {
        if (doc.node(c).kind == NodeKind::k_text)
        {
            out.append(doc.node(c).text);
        }
    }
    return out;
}
} // namespace

std::string resolve_url(std::string_view base, std::string_view ref)
{
    if (ref.empty())
    {
        return std::string(base);
    }
    if (has_scheme(ref) || (ref.size() >= 5 && ref.substr(0, 5) == "data:"))
    {
        return std::string(ref);
    }
    std::string base_str(base);
    if (ref.size() >= 2 && ref[0] == '/' && ref[1] == '/')
    {
        return std::string(scheme_of(base_str)) + ":" + std::string(ref);
    }
    std::string scheme(scheme_of(base_str));
    std::string authority(authority_of(base_str));
    std::string merged;
    if (!ref.empty() && ref[0] == '/')
    {
        merged = std::string(ref);
    }
    else
    {
        std::string base_path = base_str;
        std::size_t q = base_path.find_first_of("?#");
        if (q != std::string::npos)
        {
            base_path.resize(q);
        }
        std::size_t auth_end = base_path.find("://");
        std::string prefix;
        std::string path;
        if (auth_end != std::string::npos)
        {
            std::size_t slash = base_path.find('/', auth_end + 3);
            if (slash == std::string::npos)
            {
                prefix = base_path;
                path = "/";
            }
            else
            {
                prefix = base_path.substr(0, slash);
                path = base_path.substr(slash);
            }
        }
        else
        {
            std::size_t slash = base_path.rfind('/');
            if (slash == std::string::npos)
            {
                path = std::string(ref);
                return normalize_path(path);
            }
            prefix = "";
            path = base_path.substr(0, slash + 1) + std::string(ref);
            return normalize_path(path);
        }
        std::size_t last = path.rfind('/');
        std::string dir = (last == std::string::npos) ? "/" : path.substr(0, last + 1);
        merged = normalize_path(dir + std::string(ref));
        if (!prefix.empty())
        {
            return prefix + merged;
        }
        return merged;
    }
    if (!scheme.empty() && !authority.empty())
    {
        return scheme + "://" + authority + normalize_path(merged);
    }
    if (!scheme.empty())
    {
        return scheme + ":" + normalize_path(merged);
    }
    return normalize_path(merged);
}

DocumentBundle load_document(const std::string& html, std::string_view base_url,
                             ResourceLoader& loader)
{
    DocumentBundle bundle;
    bundle.base_url = std::string(base_url);
    bundle.dom = parse_html(html);
    Document& doc = bundle.dom;
    std::string base(bundle.base_url);
    bool base_found = false;
    std::string css;
    std::vector<int> stack;
    if (doc.root() != k_no_node)
    {
        stack.push_back(doc.root());
    }
    while (!stack.empty())
    {
        int node = stack.back();
        stack.pop_back();
        const Node& n = doc.node(node);
        if (n.kind != NodeKind::k_element)
        {
            continue;
        }
        if (n.tag == "base" && !base_found)
        {
            std::string_view href = doc.attr(node, "href");
            if (!href.empty())
            {
                base = resolve_url(bundle.base_url, href);
                base_found = true;
            }
        }
        else if (n.tag == "title" && bundle.title.empty())
        {
            bundle.title = element_text(doc, node);
        }
        else if (n.tag == "style")
        {
            std::string_view media = doc.attr(node, "media");
            if (stylesheet_media_runs(media))
            {
                css += element_text(doc, node);
                css.push_back('\n');
            }
        }
        else if (n.tag == "link" && rel_is_stylesheet(doc.attr(node, "rel")))
        {
            std::string_view href = doc.attr(node, "href");
            std::string_view media = doc.attr(node, "media");
            if (!href.empty() && stylesheet_media_runs(media))
            {
                std::string url = resolve_url(base, href);
                std::string bytes;
                std::string mime;
                if (loader.load(url, bytes, mime))
                {
                    css += bytes;
                    css.push_back('\n');
                }
            }
        }
        else if (n.tag == "script")
        {
            std::string_view src = doc.attr(node, "src");
            if (!src.empty())
            {
                if (script_type_runs(doc.attr(node, "type")))
                {
                    std::string url = resolve_url(base, src);
                    std::string bytes;
                    std::string mime;
                    if (loader.load(url, bytes, mime))
                    {
                        Script script;
                        script.text = bytes;
                        script.url = url;
                        script.external = true;
                        bundle.scripts.push_back(script);
                    }
                }
            }
            else if (script_type_runs(doc.attr(node, "type")))
            {
                Script script;
                script.text = element_text(doc, node);
                script.url = base;
                bundle.scripts.push_back(script);
            }
        }
        std::vector<int> kids;
        for (int c = n.first_child; c != k_no_node; c = doc.node(c).next_sibling)
        {
            kids.push_back(c);
        }
        for (int i = static_cast<int>(kids.size()) - 1; i >= 0; --i)
        {
            stack.push_back(kids[static_cast<std::size_t>(i)]);
        }
    }
    bundle.base_url = base;
    bundle.sheet = parse_css(css);
    return bundle;
}

} // namespace wv
