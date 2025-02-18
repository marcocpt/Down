//
//  BaseNode.swift
//  Down
//
//  Created by John Nguyen on 21.04.19.
//
//

import Foundation
import libcmark

public class BaseNode: Node {
    
    public let cmarkNode: CMarkNode
    
    public private(set) lazy var children: [Node] = {
        var result: [Node] = []
        var child = cmark_node_first_child(cmarkNode)
        
        while let raw = child {
            
            guard let node = raw.wrap() else {
                assertionFailure("Couldn't wrap node of type: \(raw.type)")
                continue
            }
            
            result.append(node)
            child = cmark_node_next(child)
        }
        
        return result
    }()
    
    public private(set) lazy var parent: Node? = {
        let rawParent = cmark_node_parent(cmarkNode)
        guard let parent = rawParent?.wrap() else { return nil }
        return parent
    }()
    
    public init(cmarkNode: CMarkNode) {
        self.cmarkNode = cmarkNode
    }
    
}
