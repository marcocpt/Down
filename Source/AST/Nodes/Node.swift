//
//  Node.swift
//  Down
//
//  Created by John Nguyen on 07.04.19.
//

import Foundation
import libcmark

/// A node is a wrapper of a raw `CMarkNode` belonging to the abstract syntax tree
/// generated by cmark.
public protocol Node {
    /// The wrapped node.
    var cmarkNode: CMarkNode { get }
    
    /// The wrapped child nodes.
    var children: [Node] { get }
    
    /// The wrapped parent node.
    var parent: Node? { get }
}

public extension Node {
    /// True if the node has a sibling that succeeds it.
    var hasSuccessor: Bool {
        return cmark_node_next(cmarkNode) != nil
    }
}

// MARK: - Helper extensions

public typealias CMarkNode = UnsafeMutablePointer<cmark_node>

public extension UnsafeMutablePointer where Pointee == cmark_node {
    
    /// Wraps the cmark node referred to by this pointer.
    func wrap() -> Node? {
        switch type {
        case CMARK_NODE_DOCUMENT:       return Document(cmarkNode: self)
        case CMARK_NODE_BLOCK_QUOTE:    return BlockQuote(cmarkNode: self)
        case CMARK_NODE_LIST:           return List(cmarkNode: self)
        case CMARK_NODE_ITEM:           return Item(cmarkNode: self)
        case CMARK_NODE_CODE_BLOCK:     return CodeBlock(cmarkNode: self)
        case CMARK_NODE_HTML_BLOCK:     return HtmlBlock(cmarkNode: self)
        case CMARK_NODE_CUSTOM_BLOCK:   return CustomBlock(cmarkNode: self)
        case CMARK_NODE_PARAGRAPH:      return Paragraph(cmarkNode: self)
        case CMARK_NODE_HEADING:        return Heading(cmarkNode: self)
        case CMARK_NODE_THEMATIC_BREAK: return ThematicBreak(cmarkNode: self)
        case CMARK_NODE_TEXT:           return Text(cmarkNode: self)
        case CMARK_NODE_SOFTBREAK:      return SoftBreak(cmarkNode: self)
        case CMARK_NODE_LINEBREAK:      return LineBreak(cmarkNode: self)
        case CMARK_NODE_CODE:           return Code(cmarkNode: self)
        case CMARK_NODE_HTML_INLINE:    return HtmlInline(cmarkNode: self)
        case CMARK_NODE_CUSTOM_INLINE:  return CustomInline(cmarkNode: self)
        case CMARK_NODE_EMPH:           return Emphasis(cmarkNode: self)
        case CMARK_NODE_STRONG:         return Strong(cmarkNode: self)
        case CMARK_NODE_LINK:           return Link(cmarkNode: self)
        case CMARK_NODE_IMAGE:          return Image(cmarkNode: self)
        
        /// extensions
        case CMARK_NODE_STRIKETHROUGH:  return Strikethrough(cmarkNode: self)
        case CMARK_NODE_TABLE:          return Table(cmarkNode: self)
        case CMARK_NODE_TABLE_ROW:      return TableRow(cmarkNode: self)
        case CMARK_NODE_TABLE_CELL:     return TableCell(cmarkNode: self)
            
        default:                        return nil
        }
    }
    
    var type: cmark_node_type {
        return cmark_node_get_type(self)
    }
    
    var literal: String? {
        return String(cString: cmark_node_get_literal(self))
    }
    
    var fenceInfo: String? {
        return String(cString: cmark_node_get_fence_info(self))
    }
    
    var headingLevel: Int {
        return Int(cmark_node_get_heading_level(self))
    }
    
    var listType: cmark_list_type {
        return cmark_node_get_list_type(self)
    }
    
    var listStart: Int {
        return Int(cmark_node_get_list_start(self))
    }
    
    var url: String? {
        return String(cString: cmark_node_get_url(self))
    }
    
    var title: String? {
        return String(cString: cmark_node_get_title(self))
    }
    
    var typeString: String? {
        return String(cString: cmark_node_get_type_string(self))
    }
}

private extension String {
    init?(cString: UnsafePointer<Int8>?) {
        guard let unwrapped = cString else { return nil }
        let result = String(cString: unwrapped)
        guard !result.isEmpty else { return nil }
        self = result
    }
}

public enum MarkdownExtension: String, CaseIterable {
    case table, strikethrough, autolink, tagfilter, tasklist
}

