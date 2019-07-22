//
//  Table.swift
//  Down
//
//  Created by marcow on 2019/7/22.
//  Copyright © 2019 Glazed Donut, LLC. All rights reserved.
//

import Foundation
import libcmark

public class Table: BaseNode {}

// MARK: - Debug

extension Table: CustomDebugStringConvertible {
    public var debugDescription: String {
        return "Table"
    }
}

public class TableRow: BaseNode {}

extension TableRow: CustomDebugStringConvertible {
    public var debugDescription: String {
        return "TableRow"
    }
}

public class TableCell: BaseNode {}

extension TableCell: CustomDebugStringConvertible {
    public var debugDescription: String {
        return "TableCell"
    }
}
