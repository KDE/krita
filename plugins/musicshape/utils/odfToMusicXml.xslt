<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:music="http://www.calligra.org/music" version="1.0">

<xsl:output method="xml" indent="yes" encoding="UTF-8"
     omit-xml-declaration="no" standalone="no"
     doctype-system="http://www.musicxml.org/dtds/partwise.dtd"
     doctype-public="-//Recordare//DTD MusicXML 2.0 Partwise//EN" />

<xsl:template match="/">
    <xsl:apply-templates select="//music:score-partwise" />
</xsl:template>

<xsl:template match="//music:score-partwise">
    <xsl:element name="{local-name()}">
        <xsl:apply-templates select="node()"/>
    </xsl:element>
</xsl:template>

<xsl:template match="*">
    <xsl:element name="{local-name()}">
      <xsl:apply-templates select="@*|node()"/>
    </xsl:element>
  </xsl:template>

  <xsl:template match="@*">
    <xsl:attribute name="{local-name()}">
      <xsl:value-of select="."/>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="processing-instruction()|comment()">
    <xsl:copy>
      <xsl:apply-templates select="node()"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
