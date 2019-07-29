//
//  ViewController.swift
//  macOS Demo
//
//  Created by Chris Zielinski on 10/27/18.
//  Copyright © 2018 down. All rights reserved.
//

import Cocoa
import Down

final class ViewController: NSViewController {

    @IBOutlet var textView: NSTextView!
    @IBOutlet weak var textViewRight: NSTextView!
    override func viewDidLoad() {
        super.viewDidLoad()

//        renderDownInWebView()
        renderDownInTextView()
        
    }
    
}

private extension ViewController {
    
    func renderDownInWebView() {
        let readMeURL = Bundle.main.url(forResource: "Test", withExtension: "md")!
        let readMeContents = try! String(contentsOf: readMeURL)
        
        do {
            let downView = try DownView(frame: view.bounds, markdownString: readMeContents, extensions:[MarkdownExtension.table, .strikethrough], didLoadSuccessfully: {
                print("Markdown was rendered.")
            })
            downView.autoresizingMask = [.width, .height]
            view.addSubview(downView, positioned: .above, relativeTo: nil)
        } catch {
            NSApp.presentError(error)
        }
    }
    
    func renderDownInTextView() {
        let readMeURL = Bundle.main.url(forResource: "Test", withExtension: "md")!
        let readMeContents = try! String(contentsOf: readMeURL)
        
        do {
          let extensions = [MarkdownExtension.strikethrough, .table, .tasklist]
          let down = Down(markdownString: readMeContents, extensions: extensions)
          let ast = try down.toAST()
          let result = Document(cmarkNode:ast).accept(DebugVisitor())
          print(result)
          let string = try down.toAttributedString(styler: MTStyler(values: StyleValues(), listPrefixAttributes: [:]))
//          let string = NSAttributedString(string: try down.toCommonMark(DownOptions.sourcePos))
//          let string = try down.toAttributedString()
            textView.textStorage?.append(string)
                        
//            textViewRight.textStorage?.append(NSAttributedString(string: readMeContents))
            textViewRight.textStorage?.append(try down.toAttributedString())
//            textViewRight.textStorage?.append(NSAttributedString(string: try down.toCommonMark(DownOptions.sourcePos)))
        } catch {
            NSApp.presentError(error)
        }
    }
}

