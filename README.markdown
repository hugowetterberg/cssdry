Css Dry
===============================

Css Dry allows you to write css without repeating yourself all the time. Take the following hypothetical example:

    #header ul {
      margin: 0; padding: 0;
      text-align: left;
    }

    #header li {
      float: left;
      padding: 0 0 0 10px;
    }

    #header #primary {
      position: absolute;
      right: 0;
      bottom: 25px;
    }

    #header #primary li {
      border-right: 1px solid #000;
      padding: 0 10px 3px 10px;
    }

    #header #primary li.last {
      border: none;
      padding: 0 0 0 10px;
    }

    #header #primary li a {
      float: left;
      height: 27px;
      text-indent: -9999px;
      outline: none;
    }

There is a lot of repetition going on in the above example. Both the #header and the #primary selector gets repeated so much that it's silly. I think that the best way to illustrate how cssdry works is to show how the same css would be expressed with cssdry.

    $border_color=#000;

    #header {
      ul { margin: 0; padding: 0; text-align: left; }
      li { float: left; padding: 0 0 0 10px; }

      #primary {
        position: absolute;
        right: 0;
        bottom: 25px;

        li {
          border-right: 1px solid $border_color;
          padding: 0 10px 3px 10px;
        }
        li.last {
          border: none;
          padding: 0 0 0 10px;
        }
        li a {
          float: left;
          height: 27px;
          text-indent: -9999px;
          outline: none;
        }
      }
    }

The above dry css generates the following css rules after processing:

    #header ul{margin: 0; padding: 0; text-align: left;}
    #header li{float: left; padding: 0 0 0 10px;}
    #header #primary{position: absolute; right: 0; bottom: 25px;}
    #header #primary li{border-right: 1px solid #000; padding: 0 10px 3px 10px;}
    #header #primary li.last{border: none; padding: 0 0 0 10px;}
    #header #primary li a{float: left; height: 27px; text-indent: -9999px; outline: none;}

For a more complete example see [this gist](http://gist.github.com/170930).