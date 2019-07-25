//
//  Down.swift
//  Down
//
//  Created by Rob Phillips on 5/28/16.
//  Copyright © 2016-2019 Glazed Donut, LLC. All rights reserved.
//

import Foundation

public struct Down: DownASTRenderable, DownHTMLRenderable, DownXMLRenderable,
                    DownLaTeXRenderable, DownGroffRenderable, DownCommonMarkRenderable,
                    DownAttributedStringRenderable {
    // DownRenderable: A string containing CommonMark Markdown
    public var markdownString: String
    
    /// DownRenderable: 使用的扩展类型
    public var markdownExtensions: [MarkdownExtension]
    
    /// Initializes the container with a CommonMark Markdown string which can then be rendered depending on protocol conformancce
    ///
    /// - Parameter
    ///     - markdownString: A string containing CommonMark Markdown
    ///     - extensions: 使用的扩展类型
    public init(markdownString: String, extensions: [MarkdownExtension] = []) {
        self.markdownString = markdownString
        self.markdownExtensions = extensions
    }
}
