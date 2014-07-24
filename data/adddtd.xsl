<?xml version="1.0" ?>
<xsl:stylesheet version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

   <xsl:template match="/">
	    <xsl:text disable-output-escaping='yes'>&lt;!DOCTYPE root-element SYSTEM "gccxml.dtd"></xsl:text>
         <xsl:copy>
               <xsl:apply-templates select="@* | node()" />
         </xsl:copy>
   </xsl:template> 	

   <!-- IdentityTransform -->
   <xsl:template match="@* | node()">
         <xsl:copy>
               <xsl:apply-templates select="@* | node()" />
         </xsl:copy>
   </xsl:template>

</xsl:stylesheet>

