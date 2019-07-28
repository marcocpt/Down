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

public class TableRow: BaseNode {
    var isHeader: Bool {
        return cmark_gfm_extensions_get_table_row_is_header(self.cmarkNode) != 0
    }
}

extension TableRow: CustomDebugStringConvertible {
    public var debugDescription: String {
        return isHeader ? "TableRow - Header" : "TableRow"
    }
}

public class TableCell: BaseNode {
    var inHeader: Bool {
        guard let parent = self.parent as? TableRow else { return false }
        return parent.isHeader
    }
}

extension TableCell: CustomDebugStringConvertible {
    public var debugDescription: String {
        return "TableCell"
    }
}
