//
//  Item.swift
//  Down
//
//  Created by John Nguyen on 09.04.19.
//

import Foundation
import libcmark

public class Item: BaseNode {
    public private(set) lazy var typeString = cmarkNode.typeString
    
    public var tasklistSelected: Bool {
        get {
            return cmark_gfm_extensions_get_tasklist_item_checked(cmarkNode)
        }
        set {
            cmark_gfm_extensions_set_tasklist_item_checked(cmarkNode, newValue)
        }
    }
}

// MARK: - Debug

extension Item: CustomDebugStringConvertible {
    public var debugDescription: String {
        if self.typeString == ItemType.tasklist.rawValue {
            return  "Item - \(tasklistSelected ? "Selected" : "UnSelected")"
        }
        return "Item"
    }
}

public enum ItemType: String {
    case item, tasklist
}
