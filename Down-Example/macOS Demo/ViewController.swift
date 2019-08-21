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
    
    let extensions = [MarkdownExtension.strikethrough, .table, .tasklist, .autolink]
    
    override func viewDidLoad() {
        super.viewDidLoad()
        textView.delegate = self
        NotificationCenter.default.addObserver(self, selector: #selector(textViewTextDidChange(_:)), name: NSText.didChangeNotification, object: self.textView)
//        renderDownInWebView()
        renderDownInTextView()
        
    }
    
    @objc func textViewTextDidChange(_ notification: NSNotification) {
        guard let view = notification.object as? NSTextView else { return }
        
        do {
          
            let down = Down(markdownString: view.string, extensions: extensions)
            let string = try down.toAttributedString(styler: MTStyler(values: StyleValues(), listPrefixAttributes: [:]))
            textView.textStorage?.deleteCharacters(in: NSMakeRange(0, view.string.count))
            textView.textStorage?.append(string)

        }  catch {
            NSApp.presentError(error)
        }
    }
    
}

private extension ViewController {
    
    func renderDownInWebView() {
        let readMeURL = Bundle.main.url(forResource: "Test", withExtension: "md")!
        let readMeContents = try! String(contentsOf: readMeURL)
        
        do {
            let downView = try DownView(frame: view.bounds, markdownString: readMeContents, extensions:extensions, didLoadSuccessfully: {
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

extension ViewController: NSTextViewDelegate {
    func textView(_ textView: NSTextView, shouldChangeTextIn affectedCharRange: NSRange, replacementString: String?) -> Bool {
        return true
    }
}
