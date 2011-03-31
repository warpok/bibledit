<?php
require_once 'PHPUnit/Framework.php';
 
class filterTextTest extends PHPUnit_Framework_TestCase
{

private $temporary_folder;

  public function setUp ()
  {
    $this->tearDown ();
  }
  
  public function tearDown ()
  {

  }

  
  /**
  * Test extraction of all sorts of information from USFM code
  * Test basic formatting into OpenDocument.
  */
  public function testOne()
  {
$usfm = <<<'EOD'
\id GEN
\h Header
\h1 Header1
\h2 Header2
\h3 Header3
\toc1 The Book of Genesis
\toc2 Genesis
\toc3 Gen
\cl Chapter
\c 1
\cp Ⅰ
\p Text chapter 1
\c 2
\cp ①
\h Header4
\p Text chapter 2
EOD;
    $filter_text = new Filter_Text;
    $filter_text->addUsfmCode ($usfm);
    $filter_text->run ("/tmp/TextTest1.odt");
    // Check that it finds the running headers.
    $this->assertEquals (array ('book' => 1, 'chapter' => 0, 'verse' => 0, 'marker' => 'h', 'value' => 'Header'),  $filter_text->runningHeaders[0]);
    $this->assertEquals (array ('book' => 1, 'chapter' => 0, 'verse' => 0, 'marker' => 'h1', 'value' => 'Header1'), $filter_text->runningHeaders[1]);
    $this->assertEquals (array ('book' => 1, 'chapter' => 0, 'verse' => 0, 'marker' => 'h2', 'value' => 'Header2'), $filter_text->runningHeaders[2]);
    $this->assertEquals (array ('book' => 1, 'chapter' => 0, 'verse' => 0, 'marker' => 'h3', 'value' => 'Header3'), $filter_text->runningHeaders[3]);
    $this->assertEquals (array ('book' => 1, 'chapter' => 2, 'verse' => 0, 'marker' => 'h', 'value' => 'Header4'), $filter_text->runningHeaders[4]);
    // Check on Table of Contents items.
    $this->assertEquals (array ('book' => 1, 'chapter' => 0, 'verse' => 0, 'marker' => 'toc1', 'value' => 'The Book of Genesis'), $filter_text->longTOCs[0]);
    $this->assertEquals (array ('book' => 1, 'chapter' => 0, 'verse' => 0, 'marker' => 'toc2', 'value' => 'Genesis'), $filter_text->shortTOCs[0]);
    // Check book abbreviation.
    $this->assertEquals (array ('book' => 1, 'chapter' => 0, 'verse' => 0, 'marker' => 'toc3', 'value' => 'Gen'), $filter_text->bookAbbreviations[0]);
    // Check chapter specials.
    $this->assertEquals (array ('book' => 1, 'chapter' => 0, 'verse' => 0, 'marker' => 'cl', 'value' => 'Chapter'), $filter_text->chapterLabels[0]);
    $this->assertEquals (array ('book' => 1, 'chapter' => 1, 'verse' => 0, 'marker' => 'cp', 'value' => 'Ⅰ'), $filter_text->publishedChapterMarkers[0]);
    $this->assertEquals (array ('book' => 1, 'chapter' => 2, 'verse' => 0, 'marker' => 'cp', 'value' => '①'), $filter_text->publishedChapterMarkers[1]);
    $this->assertEquals (array (1 => 2), $filter_text->numberOfChaptersPerBook);
    $javaCode = $filter_text->odfdom_text_standard->javaCode;
    $dir = Filter_Java::compile ($javaCode, array (Odfdom_Class::path (), Filter_Java::xercesClassPath()));
    $return_var = Filter_Java::run ($dir, array (Odfdom_Class::path (), Filter_Java::xercesClassPath()), "odt");
    $this->assertEquals (0, $return_var);
    exec ("odt2txt /tmp/TextTest1.odt", $output, &$return_var);
    $this->assertEquals (array ("", "Text chapter 1", "", "Text chapter 2", ""), $output);
  }

  /**
  * There are two books here. This normally gives one new page between these two books.
  * Test that basic USFM code gets transformed correctly.
  */
  public function testTwo()
  {
$usfm = <<<'EOD'
\id GEN
\ide XYZ
\c 1
\p Text Genesis 1
\c 2
\p Text Genesis 2
\id MAT
\c 1
\p Text Matthew 1
\c 2
\p Text Matthew 2
\rem Comment
\xxx Unknown markup
EOD;
    $filter_text = new Filter_Text;
    $filter_text->addUsfmCode ($usfm);
    $filter_text->run ("/tmp/TextTest2.odt");
    $javaCode = $filter_text->odfdom_text_standard->javaCode;
    $dir = Filter_Java::compile ($javaCode, array (Odfdom_Class::path (), Filter_Java::xercesClassPath()));
    $return_var = Filter_Java::run ($dir, array (Odfdom_Class::path (), Filter_Java::xercesClassPath()), "odt");
    $this->assertEquals (0, $return_var);
    exec ("odt2txt /tmp/TextTest2.odt", $output, &$return_var);
    $this->assertEquals (array ("", "Text Genesis 1", "", "Text Genesis 2", "", "Text Matthew 1", "", "Text Matthew 2", ""), $output);
    $this->assertEquals (array ('Genesis 0:0 Text encoding indicator not supported. Encoding is always in UTF8: \ide XYZ',
                                'Matthew 2:0 Unknown marker \xxx Unknown markup'), $filter_text->fallout);
    $this->assertEquals (array ('Matthew 2:0 Comment: \rem Comment'), $filter_text->info);
  }



}
?>


