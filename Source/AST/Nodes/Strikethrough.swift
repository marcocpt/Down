//
//  Strikethrough.swift
//  Down
//
//  Created by marcow on 2019/7/22.
//  Copyright © 2019 Glazed Donut, LLC. All rights reserved.
//

import Foundation
import libcmark

public class Strikethrough: BaseNode {}

// MARK: - Debug

extension Strikethrough: CustomDebugStringConvertible {
    public var debugDescription: String {
        return "Strikethrough"
    }
}
