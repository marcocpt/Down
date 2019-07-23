//
//  DownASTRenderable.swift
//  Down
//
//  Created by Rob Phillips on 5/31/16.
//  Copyright © 2016-2019 Glazed Donut, LLC. All rights reserved.
//

import Foundation
import libcmark

public protocol DownASTRenderable: DownRenderable {
    func toAST(_ options: DownOptions) throws -> UnsafeMutablePointer<cmark_node>
}

extension DownASTRenderable {
    /// Generates an abstract syntax tree from the `markdownString` property
    ///
    /// - Parameter options: `DownOptions` to modify parsing or rendering, defaulting to `.default`
    /// - Returns: An abstract syntax tree representation of the Markdown input
    /// - Throws: `MarkdownToASTError` if conversion fails
    public func toAST(_ options: DownOptions = .default) throws -> UnsafeMutablePointer<cmark_node> {
        return try DownASTRenderer.stringToAST(markdownString, options: options, extensions: markdownExtensions)
    }
}

public struct DownASTRenderer {
    /// Generates an abstract syntax tree from the given CommonMark Markdown string
    ///
    /// **Important:** It is the caller's responsibility to call `cmark_node_free(ast)` on the returned value
    ///
    /// - Parameters:
    ///   - string: A string containing CommonMark Markdown
    ///   - options: `DownOptions` to modify parsing or rendering, defaulting to `.default`
    ///   - extensions: 支持的扩展类型
    /// - Returns: An abstract syntax tree representation of the Markdown input
    /// - Throws: `MarkdownToASTError` if conversion fails
    public static func stringToAST(_ string: String, options: DownOptions = .default, extensions: [MarkdownExtension]) throws -> UnsafeMutablePointer<cmark_node> {
        var tree: UnsafeMutablePointer<cmark_node>?
        cmark_gfm_core_extensions_ensure_registered()
        
        guard let parser = cmark_parser_new(options.rawValue) else {
            throw DownErrors.markdownToASTError
        }
        defer { cmark_parser_free(parser) }
        
        extensions
            .compactMap { cmark_find_syntax_extension($0.rawValue) }
            .forEach { cmark_parser_attach_syntax_extension(parser, $0) }
        
        string.withCString {
            let stringLength = Int(strlen($0))
            cmark_parser_feed(parser, $0, stringLength)
            tree = cmark_parser_finish(parser)
        }

        guard let ast = tree else {
            throw DownErrors.markdownToASTError
        }
        return ast
    }
}
