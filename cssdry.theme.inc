<?php

/**
 * When the theme registry is rebuilt, we also build the cssdry stylesheets.
 */
function _cssdry_theme($existing, $type, $theme, $path) {  
  // Process the stylesheets for every active theme.
  global $language;
  $themes = list_themes(); 
  
  foreach (array_keys($themes) AS $theme) {
    // We only need to process active themes.
    if (!$themes[$theme]->status) {
      continue;
    }
    
    $paths = cssdry_paths_to_basetheme($theme);

    // Grab all the cssdry stylesheets.
    $stylesheets = array();
    // Start with the base theme and travel up the chain to the active theme.
    foreach ($paths AS $theme_name => $path) {
      // Look at the conditional-stylesheets defined in the theme's .info file.
      if (!empty($themes[$theme_name]->info['cssdry'])) {
        foreach ($themes[$theme_name]->info['cssdry'] AS $media => $files) {
          // Allow the theme to override its base themes' styles.            
          foreach ($files AS $file) {
            $stylesheets[$media][$file] = $path;
          }            
        }
      }
    }           
    
    // Render the stylesheets to link elements.
    $cssdry_styles = '';
    if (!empty($stylesheets)) {
      $query_string = '?'. substr(variable_get('css_js_query_string', '0'), 1, 1);
      $base_path = base_path();
      $output = '';
      foreach ($stylesheets AS $media => $files) {                             
        foreach ($files AS $file => $path) {
          // Don't allow non-existent stylesheets to clutter the logs with 404.
          if (file_exists("./$path/$file")) {                       
            $cssdry_path = cssdry_stylesheet("./$path/$file", $base_path);              
            $cssdry_styles .= "<link type=\"text/css\" rel=\"stylesheet\" media=\"$media\" href=\"$base_path$cssdry_path$query_string\" />\n";
            if ($language->direction == LANGUAGE_RTL){
              $file_rtl = str_replace('.css', '-rtl.css', $file);                 
              if (file_exists("./$path/$file_rtl")) {
                $cssdry_path_rtl = cssdry_stylesheet("./$path/$file_rtl", $base_path);
                $output .= "<link type=\"text/css\" rel=\"stylesheet\" media=\"$media\" href=\"$base_path$cssdry_path_rtl$query_string\" />\n";
              }
            }
          }
        }
      }
    }
    // Save the cssdry header section for later retrieval.
    if ($cssdry_styles) {
      variable_set('cssdry_' . $theme, $cssdry_styles);
    }
    else {
      variable_del('cssdry_' . $theme);
    }  
  }

  // Return nothing.
  return array();
}

