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
}

// MARK: - Debug

extension Item: CustomDebugStringConvertible {
    public var debugDescription: String {
        return "Item - \(typeString ?? "nil")"
    }
}

public enum ItemType: String {
    case item, tasklist
}
