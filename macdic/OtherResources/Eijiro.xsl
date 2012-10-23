<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
				xmlns:d="http://www.apple.com/DTDs/DictionaryService-1.0.rng"
				version="1.0">
<xsl:output method="xml" encoding="UTF-8" indent="no"
	doctype-public="-//W3C//DTD XHTML 1.1//EN"
	doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd" />

<!--
    targets

<xsl:template match="*[@dic='01']">
	<xsl:if test="$target-eiji = '1'">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()" />
		</xsl:copy>
	</xsl:if>
</xsl:template>

<xsl:template match="*[@dic='02']">
	<xsl:if test="$target-waei = '1'">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()" />
		</xsl:copy>
	</xsl:if>
</xsl:template>

<xsl:template match="*[@dic='03']">
	<xsl:if test="$target-reiji = '1'">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()" />
		</xsl:copy>
	</xsl:if>
</xsl:template>

<xsl:template match="*[@dic='04']">
	<xsl:if test="$target-reijitag = '1'">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()" />
		</xsl:copy>
	</xsl:if>
</xsl:template>

<xsl:template match="*[@dic='05']">
	<xsl:if test="$target-ryaku = '1'">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()" />
		</xsl:copy>
	</xsl:if>
</xsl:template>
-->
<!--
	Default rule for all other tags
-->
<xsl:template match="@*|node()">
	<xsl:copy>
		<xsl:apply-templates select="@*|node()" />
	</xsl:copy>
</xsl:template>

</xsl:stylesheet>
