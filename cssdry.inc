<?php
// $Id$

/**
 * @file
 *  CSSDRY:ing code from:
 *  http://www.machete.ca/2009/06/more-powerful-css-with-preprocessing/
 */

function _cssdry($raw_css, $base_path) {
  $cssProcessor = new CSSProcessor($raw_css, $base_path);
  return $cssProcessor->process();
}

class CSSProcessor {
  private $base_path = '';
  private $constants = array();
  private $content;

  public function __construct($content, $base_path) {
    $this->content = $content;
    $this->base_path = $base_path;
  }

  /**
   * Processes CSS so that:
   * 1. imports are expanded inline
   *      @import url("testing.css") would be loaded in place
   *      and any imports it has would also be imported
   * 2. constants may be defined.  For example [Background Color=#000]
   *      would allow for the use of [Background Color] instead of the hard coded #000
   * 3. Single line comments are allows ala //this is a comment style
   * 4. The following is stripped out: comments, unnecessary whitespace, empty rules (a {})
   * 5. Nesting of CSS rules.
   *     For example #navbar a { ... } could be rewritten as #navbar { a{ ... } }
   */
  public function process() {
    $this->expandImports();
    $this->applyBasePath();
    $this->expandConstants();
    $this->fixNesting();
    $this->minify();

    return $this->content;
  }

  private function expandConstants() {
    $css = preg_replace_callback(
      '/\$([a-z_-]+)+=([^;]+);?/',
      array($this, 'extractConstantsCallback'),
      $this->content);

    $css = preg_replace_callback(
      '/\$([a-z_-]+)/',
      array($this, 'substituteConstantsCallback'),
      $css);

    $this->content = $css;
  }

  private function extractConstantsCallback(array $matches) {
    $constant = $matches[1];
    $value = $matches[2];
    $this->constants[$constant] = $value;
    return '';
  }

  private function substituteConstantsCallback(array $matches) {
    $constant_name = $matches[1];

    $value = '';
    if (!isset($this->constants[$constant_name])) {
      $value = "/* Unknown constant {$constant_name} */";
    }
    else {
      $value = $this->constants[$constant_name];
    }

    return $value;
  }

  private function expandImports($max_depth = 10) {
    $cssText = $this->content;
    $imported = $cssText;
    $import_depth = 0;
    do {
      $import_depth ++;

      $cssText = $imported;
      $imported = preg_replace_callback(
        '#@import +url\("([^"]+\.css)\"\);?#',
        array($this, 'importCallback'),
        $cssText);
    } while ($imported != $cssText && $import_depth < 10);

    if ($import_depth == $max_depth) {
      $this->content = '/*Recursive import problem*/';
    } else {
      $this->content = $cssText;
    }
  }

  private function importCallback(array $matches) {
    $filename = $matches[1];

    return file_get_contents($this->base_path.'/'.$filename);
  }

  private function applyBasePath() {
    $this->content = preg_replace_callback('/url\(([^)]+)\)/', array($this, 'basePathCallback'), $this->content);
  }

  public function basePathCallback($match) {
    if (!preg_match('/(^[a-z]+:\/\/)|^\//', $match[1])) {
      $ref = sprintf('url(/%s/%s)', $this->base_path, $match[1]);
    }
    else {
      $ref = $match[0];
    }
    return $ref;
  }

  private function fixNesting() {
    $result = array();
    $pieces = split("{", $this->content);

    $selectorStackIndex = 0;
    $selectorStack = array();

    $currentPieceIndex = 0;

    for ($i = 0; $i<count($pieces); $i++) {
      $piece = $pieces[$i];
      while (($closeBracketPos = strpos($piece, "}")) !== false) {
        if ($closeBracketPos > 0) {
          $result[] = substr($piece, 0, $closeBracketPos);;
        }

        $result[] = "}";

        $selectorStackIndex--;

        if ($selectorStackIndex > 0 ) {
          $result[] = $this->fixNestingSelector($selectorStack, $selectorStackIndex);
          $result[] = "{";
        }
        $piece = substr($piece, $closeBracketPos+1);
      }

      if (trim($piece) === '')
        continue;

      // Inner Rule
      $endOfLastProperty = strrpos($piece, ";");
      if ($endOfLastProperty !== false) {
        $result[] = substr($piece, 0, $endOfLastProperty+1);
        $piece = substr($piece, $endOfLastProperty+1);
      }

      if ($selectorStackIndex > 0)
        $result[] = "}";

      // Whole piece is the selector
      $selector = $piece;

      $selectorStack[$selectorStackIndex++] = $selector;
      $result[] = $this->fixNestingSelector($selectorStack, $selectorStackIndex);
      $result[] = '{';
    }

    $this->content = join($result);
  }

  private function fixNestingSelector($selectors, $depth) {
    $newSelector = $selectors[0];

    for($j = 1; $j<$depth; $j++)
      $newSelector = $this->fixNestingBuildSelector($newSelector, $selectors[$j]);

    return $newSelector;
  }

  private function fixNestingBuildSelector($outer, $inner) {
    $outerPieces = split(",", $outer);
    $innerPieces = split(",", $inner);

    $resultPieces = array();
    foreach ($outerPieces as $o) {
      foreach ($innerPieces as $i) {
        $resultPieces[] = trim($o) . " " . trim($i);
      }
    }

    return join(",", $resultPieces);
  }

  /**
   * Minifies the CSS content by removing things that are unnecessary
   */
  private function minify() {
    // remove single line comments, like this, from // to \\n
    $data = preg_replace('/(\/\/.*\n)/', '', $this->content);

    // remove new lines \\n, tabs and \\r
    $data = preg_replace('/(\t|\r|\n)/', '', $data);

    // remove multi-line comments
    $data = preg_replace('/(\/\*[^*]*\*\/)/', '', $data);

    // remove multi-line comments
    $data = preg_replace('/(\/\*[^\/]*\*\/)/', '', $data);

    // replace multi spaces with singles
    $data = preg_replace('/(\s+)/', ' ', $data);

    //Remove empty rules
    $data = preg_replace('/[^}{]+{\s?}/', '', $data);

    // Remove whitespace around selectors and braces
    $data = preg_replace('/\s*{\s*/', '{', $data);

    // Remove whitespace at end of rule
    $data = preg_replace('/\s*}\s*/', '}', $data);

    // Just for clarity, make every rules 1 line tall
    $data = preg_replace('/}/', "}\n", $data);

    $this->content = $data;
  }

}