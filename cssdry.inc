<?php
// $Id$

/**
 * @file
 *  CSSDRY:ing code from:
 *  http://www.machete.ca/2009/06/more-powerful-css-with-preprocessing/
 */

function _cssdry($raw_css) {
  $cssProcessor = new CSSProcessor($raw_css);
  return $cssProcessor->process();
}

class CSSProcessor {
  private $constants = array();
  private $content;

  public function __construct($content) {
    $this->content = $content;
  }

  /**
   * Processes CSS so that:
   * 
   * 1. constants may be defined.  For example $background=#000
   *      would allow for the use of $background instead of the hard coded #000  
   * 2. Nesting of CSS rules.
   *     For example #navbar a { ... } could be rewritten as #navbar { a{ ... } }
   */
  public function process() {
    $this->expandConstants();
    $this->fixNesting();    

    return $this->content;
  }

  private function expandConstants() {
    $css = preg_replace_callback(
      '/\$([a-z_\-0-9]+)+=([^;]+);?/',
      array($this, 'extractConstantsCallback'),
      $this->content);

    $css = preg_replace_callback(
      '/\$([a-z_\-0-9]+)/',
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
}