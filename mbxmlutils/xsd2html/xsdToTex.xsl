<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xs="http://www.w3.org/2001/XMLSchema"
  xmlns:html="http://www.w3.org/1999/xhtml"
  version="1.0">

  <xsl:param name="PROJECT"/>
  <xsl:param name="PHYSICALVARIABLEHTMLDOC"/>
  <xsl:param name="INCLUDEDOXYGEN"/>

  <xsl:template name="CONUNDERSCORE">
    <xsl:param name="V"/>
    <xsl:if test="not(contains($V,'_'))">
      <xsl:value-of select="$V"/>
    </xsl:if>
    <xsl:if test="contains($V,'_')">
      <xsl:value-of select="concat(substring-before($V,'_'),'\_',substring-after($V,'_'))"/>
    </xsl:if>
  </xsl:template>



  <!-- output method -->
  <xsl:output method="text"/>

  <!-- no default text -->
  <xsl:template match="text()"/>



  <xsl:template match="/">
<!-- tex header -->
\documentclass[a4]{report}
\usepackage{color}
\usepackage{graphicx}
\usepackage[utf8x]{inputenc}
\usepackage{colortbl}
\usepackage{bold-extra}
\usepackage{longtable}
\usepackage{tabularx}
\setlength{\parskip}{1em}
\setlength{\parindent}{0mm}

\pagestyle{headings}
\setlength{\hoffset}{-25.4mm}
\setlength{\oddsidemargin}{30mm}
\setlength{\evensidemargin}{30mm}
\setlength{\textwidth}{153mm}
\setlength{\voffset}{-25.4mm}
\setlength{\topmargin}{25mm}
\setlength{\headheight}{5mm}
\setlength{\headsep}{8mm}
\setlength{\textheight}{235.5mm}
\setlength{\footskip}{13mm}

\setcounter{secnumdepth}{5}
\setcounter{tocdepth}{5}
\begin{document}

\title{<xsl:value-of select="$PROJECT"/> - XML Documentation}
\maketitle

\tableofcontents
\chapter{Introduction}
    <xsl:apply-templates mode="CLASSANNOTATION" select="/xs:schema/xs:annotation/xs:documentation"/>

\chapter{Nomenclature}
\label{nomenclature}

\section{A element}
\begin{tabular}{l}
  \textbf{\texttt{$&lt;$ElementName$&gt;$}} \textit{[0-2]} (Type: \texttt{elementType})\\
  \hspace{2ex}\textbf{attrName1} \textit{[required]} (Type: \texttt{typeOfTheAttribute})\\
  \hspace{2ex}\textbf{attrName2} \textit{[optional]} (Type: \texttt{typeOfTheAttribute})\\
  \hspace{3ex}
  \begin{minipage}[b]{\linewidth}
    Documentation of the element.
  \end{minipage}
\end{tabular}

The upper nomenclature defines a XML element named \texttt{ElementName} with (if given) a minimal occurance of 0 and a maximal occurance of 2. The element is of type \texttt{elementType}.\\
A occurance of \textit{[optional]} means \textit{[0-1]}.\\
The element has two attributes named \texttt{attrName1} and \texttt{attrName2} of type \texttt{typeOfTheAttribute}. A attribute can be optional or required.

\section{A choice of elment}
\begin{tabular}{!{\color{blue}\vline}@{\hspace{0.5pt}}l}
  {\color{blue}\textit{[1-2]}}\\
  \textbf{\texttt{$&lt;$ElementA$&gt;$}}\\
  \textbf{\texttt{$&lt;$ElementB$&gt;$}}\\
\end{tabular}

The upper nomenclature defines a choice of elements. Only one element of the given ones can be used. The choice has, if given, a minimal occurance of 1 and a maximal maximal occurence of 2.\\
A occurance of \textit{[optional]} means \textit{[0-1]}.

\section{A sequence of elments}
\begin{tabular}{!{\color{red}\vline}@{\hspace{0.5pt}}l}
  {\color{red}\textit{[1-2]}}\\
  \textbf{\texttt{$&lt;$ElementA$&gt;$}}\\
  \textbf{\texttt{$&lt;$ElementB$&gt;$}}\\
\end{tabular}

The upper nomenclature defines a sequence of elements. Each element must be given in that order. The sequence has, if given, a minimal occurance of 0 and a maximal maximal occurence of 3.\\
A occurance of \textit{[optional]} means \textit{[0-1]}.

\section{Nested sequences/choices}
\begin{tabular}{!{\color{red}\vline}@{\hspace{0.5pt}}l}
  {\color{red}\textit{[1-2]}}\\
  \textbf{\texttt{$&lt;$ElementA$&gt;$}}\\
  \begin{tabular}{!{\color{blue}\vline}@{\hspace{0.5pt}}l}
    {\color{blue}\textit{[0-3]}}\\
    \textbf{\texttt{$&lt;$ElementC$&gt;$}}\\
    \textbf{\texttt{$&lt;$ElementD$&gt;$}}\\
  \end{tabular}\\
  \textbf{\texttt{$&lt;$ElementB$&gt;$}}\\
\end{tabular}

Sequences and choices can be nested like above.

\section{Child Elements}
\begin{tabular}{!{\color{red}\vline}@{\hspace{0.5pt}}l}
  {\color{red}\textit{[1-2]}}\\
  \textbf{\texttt{$&lt;$ParentElement$&gt;$}}\\
  \hspace{5ex}
  \begin{tabular}{!{\color{blue}\vline}@{\hspace{0.5pt}}l}
    {\color{blue}\textit{[0-3]}}\\
    \textbf{\texttt{$&lt;$ChildElementA$&gt;$}}\\
    \textbf{\texttt{$&lt;$ChildElementB$&gt;$}}\\
  \end{tabular}\\
\end{tabular}

A indent indicates child elements for a given element.

\chapter{Elements}
    <xsl:apply-templates mode="WALKCLASS" select="/xs:schema/xs:element[not(@substitutionGroup)]">
      <xsl:with-param name="LEVEL" select="0"/>
      <xsl:sort select="@name"/>
    </xsl:apply-templates>

\end{document}
  </xsl:template>

  <!-- walk throw all elements -->
  <xsl:template mode="WALKCLASS" match="/xs:schema/xs:element">
    <xsl:param name="LEVEL"/>
    <xsl:param name="NAME" select="@name"/>
    <xsl:apply-templates mode="CLASS" select=".">
      <xsl:with-param name="LEVEL" select="$LEVEL"/>
    </xsl:apply-templates>
    <xsl:apply-templates mode="WALKCLASS" select="/xs:schema/xs:element[@substitutionGroup=$NAME]">
      <xsl:with-param name="LEVEL" select="$LEVEL+1"/>
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- class -->
  <xsl:template mode="CLASS" match="/xs:schema/xs:element">
    <xsl:param name="LEVEL"/>
    <xsl:param name="TYPENAME" select="@type"/>
    <xsl:param name="CLASSNAME" select="@name"/>
    <!-- heading -->
    <xsl:choose>
      <xsl:when test='$LEVEL=0'>\section</xsl:when>
      <xsl:when test='$LEVEL=1'>\subsection</xsl:when>
      <xsl:when test='$LEVEL=2'>\subsubsection</xsl:when>
      <xsl:when test='$LEVEL=3'>\paragraph</xsl:when>
      <xsl:otherwise>\subparagraph</xsl:otherwise>
    </xsl:choose>{\textbf{\texttt{$&lt;$<xsl:call-template name="CONUNDERSCORE"><xsl:with-param name="V" select="@name"/></xsl:call-template>$&gt;$}}}
    \label{<xsl:value-of select="@name"/>}
    \makebox{}\\
    \setlength{\arrayrulewidth}{0.5pt}
    \begin{tabularx}{\textwidth}{|l|X|}
      \hline
      <!-- abstract -->
      Abstract Element: &amp;
      <xsl:if test="@abstract='true'">true</xsl:if>
      <xsl:if test="@abstract!='true' or not(@abstract)">false</xsl:if>\\
      \hline
      <!-- inherits -->
      Inherits: &amp;
      <xsl:if test="@substitutionGroup">
        \textbf{\texttt{$&lt;$<xsl:call-template name="CONUNDERSCORE"><xsl:with-param name="V" select="@substitutionGroup"/></xsl:call-template>$&gt;$}}
        (\ref{<xsl:value-of select="@substitutionGroup"/>}, p. \pageref{<xsl:value-of select="@substitutionGroup"/>})
      </xsl:if>\\
      \hline
      <!-- inherited by -->
      Inherited by: &amp;
      <xsl:if test="count(/xs:schema/xs:element[@substitutionGroup=$CLASSNAME])>0">
        <xsl:for-each select="/xs:schema/xs:element[@substitutionGroup=$CLASSNAME]">
          <xsl:sort select="@name"/>\textbf{\texttt{$&lt;$<xsl:call-template name="CONUNDERSCORE"><xsl:with-param name="V" select="@name"/></xsl:call-template>$&gt;$}}~(\ref{<xsl:value-of select="@name"/>},~p.~\pageref{<xsl:value-of select="@name"/>}),
        </xsl:for-each>
      </xsl:if>\\
      \hline
      <!-- used in -->
      Can be used in: &amp;
      <xsl:apply-templates mode="USEDIN2" select="."/>\\
      \hline
      <!-- class attributes -->
      Attributes: 
      <xsl:apply-templates mode="CLASSATTRIBUTE" select="/xs:schema/xs:complexType[@name=$TYPENAME]/xs:attribute"/>
      <xsl:if test="not(/xs:schema/xs:complexType[@name=$TYPENAME]/xs:attribute)"> &amp; \\</xsl:if>
      \hline
    \end{tabularx}
    \setlength{\arrayrulewidth}{1.25pt}
    <!-- class documentation -->
    <xsl:apply-templates mode="CLASSANNOTATION" select="xs:annotation/xs:documentation"/>
    <!-- child elements -->
    <xsl:if test="/xs:schema/xs:complexType[@name=$TYPENAME]/xs:complexContent/xs:extension/xs:sequence|/xs:schema/xs:complexType[@name=$TYPENAME]/xs:complexContent/xs:extension/xs:choice|/xs:schema/xs:complexType[@name=$TYPENAME]/xs:sequence|/xs:schema/xs:complexType[@name=$TYPENAME]/xs:choice"><xsl:text>

      Child Elements:
       
      </xsl:text>
      <!-- child elements for not base class -->
      <xsl:apply-templates mode="CLASS" select="/xs:schema/xs:complexType[@name=$TYPENAME]/xs:complexContent/xs:extension">
        <xsl:with-param name="CLASSNAME" select="$CLASSNAME"/>
      </xsl:apply-templates>
      <!-- child elements for base class -->
      <xsl:if test="/xs:schema/xs:complexType[@name=$TYPENAME]/xs:sequence|/xs:schema/xs:complexType[@name=$TYPENAME]/xs:choice">
        <xsl:apply-templates mode="SIMPLECONTENT" select="/xs:schema/xs:complexType[@name=$TYPENAME]/xs:sequence|/xs:schema/xs:complexType[@name=$TYPENAME]/xs:choice">
          <xsl:with-param name="CLASSNAME" select="$CLASSNAME"/>
          <xsl:with-param name="FIRST" select="'true'"/>
        </xsl:apply-templates>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <!-- class attributes -->
  <xsl:template mode="CLASSATTRIBUTE" match="/xs:schema/xs:complexType/xs:attribute">
    &amp; \textbf{\texttt{<xsl:value-of select="@name"/>}}
    <xsl:if test="@use='required'">\textit{[required]}</xsl:if>
    <xsl:if test="@use!='required'">\textit{[optional]}</xsl:if>
    (Type: \texttt{<xsl:value-of select="@type"/>})\\
  </xsl:template>

  <!-- used in -->
  <xsl:template mode="USEDIN2" match="/xs:schema/xs:element">
    <xsl:param name="SUBSTGROUP" select="@substitutionGroup"/>
    <xsl:param name="CLASSNAME" select="@name"/>
    <xsl:apply-templates mode="USEDIN" select="/descendant::xs:element[@ref=$CLASSNAME]"/>
    <xsl:apply-templates mode="USEDIN2" select="/xs:schema/xs:element[@name=$SUBSTGROUP]"/>
  </xsl:template>
  <xsl:template mode="USEDIN" match="xs:element">
    <xsl:apply-templates mode="USEDIN" select="ancestor::xs:complexType[last()]"/>
  </xsl:template>
  <xsl:template mode="USEDIN" match="xs:complexType">
    <xsl:param name="CLASSTYPE" select="@name"/>\textbf{\texttt{$&lt;$<xsl:call-template name="CONUNDERSCORE"><xsl:with-param name="V" select="/xs:schema/xs:element[@type=$CLASSTYPE]/@name"/></xsl:call-template>$&gt;$}}~(\ref{<xsl:value-of select="/xs:schema/xs:element[@type=$CLASSTYPE]/@name"/>},~p.~\pageref{<xsl:value-of select="/xs:schema/xs:element[@type=$CLASSTYPE]/@name"/>}),
  </xsl:template>

  <!-- child elements for not base class -->
  <xsl:template mode="CLASS" match="xs:extension">
    <xsl:param name="CLASSNAME"/>
    <xsl:if test="xs:sequence|xs:choice">
      <!-- elements from base class -->
      All Elements from \textbf{\texttt{$&lt;$<xsl:call-template name="CONUNDERSCORE"><xsl:with-param name="V" select="/xs:schema/xs:element[@name=$CLASSNAME]/@substitutionGroup"/></xsl:call-template>$&gt;$}}~(\ref{<xsl:value-of select="/xs:schema/xs:element[@name=$CLASSNAME]/@substitutionGroup"/>},~p.~\pageref{<xsl:value-of select="/xs:schema/xs:element[@name=$CLASSNAME]/@substitutionGroup"/>})\\
      <!-- elements from this class -->
      <xsl:apply-templates mode="SIMPLECONTENT" select="xs:sequence|xs:choice">
        <xsl:with-param name="CLASSNAME" select="$CLASSNAME"/>
        <xsl:with-param name="FIRST" select="'true'"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>



  <!-- child elements -->
  <xsl:template mode="SIMPLECONTENT" match="xs:complexType">
    <xsl:if test="xs:sequence|xs:choice">
      \hspace{5ex}
        <xsl:apply-templates mode="SIMPLECONTENT" select="xs:sequence|xs:choice"/>
    </xsl:if>
  </xsl:template>

  <!-- occurance -->
  <xsl:template mode="OCCURANCE" match="xs:sequence|xs:choice|xs:element">
    <xsl:param name="ELEMENTNAME"/>
    <xsl:param name="COLOR"/>
    <xsl:if test="@minOccurs|@maxOccurs">
      {<xsl:if test="$COLOR!=''">\color{<xsl:value-of select="$COLOR"/>}</xsl:if>
      <xsl:if test="@minOccurs=0 and not(@maxOccurs)">
        \textit{[optional]}
      </xsl:if>
      <xsl:if test="not(@minOccurs=0 and not(@maxOccurs))">
        \textit{[<xsl:if test="@minOccurs"><xsl:value-of select="@minOccurs"/></xsl:if><xsl:if test="not(@minOccurs)">1</xsl:if>-<xsl:if test="@maxOccurs"><xsl:value-of select="@maxOccurs"/></xsl:if><xsl:if test="not(@maxOccurs)">1</xsl:if>]}
      </xsl:if>}
      <xsl:if test="$ELEMENTNAME='li'">
        \\
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <!-- sequence -->
  <xsl:template mode="SIMPLECONTENT" match="xs:sequence">
    <xsl:param name="CLASSNAME"/>
    <xsl:param name="FIRST"/>
    <xsl:if test="$FIRST='true'">\begin{longtable}[l]{!{\color{red}\vline}@{\hspace{0.5pt}}l}</xsl:if>
    <xsl:if test="$FIRST!='true'">\begin{tabular}{!{\color{red}\vline}@{\hspace{0.5pt}}l}</xsl:if>
      <xsl:apply-templates mode="OCCURANCE" select=".">
        <xsl:with-param name="ELEMENTNAME" select="'li'"/>
        <xsl:with-param name="COLOR" select="'red'"/>
      </xsl:apply-templates>
      <xsl:apply-templates mode="SIMPLECONTENT" select="xs:element|xs:sequence|xs:choice">
        <xsl:with-param name="CLASSNAME" select="$CLASSNAME"/>
      </xsl:apply-templates>
    <xsl:if test="$FIRST='true'">\end{longtable}</xsl:if>
    <xsl:if test="$FIRST!='true'">\end{tabular}\\</xsl:if>
  </xsl:template>

  <!-- choice -->
  <xsl:template mode="SIMPLECONTENT" match="xs:choice">
    <xsl:param name="CLASSNAME"/>
    <xsl:param name="FIRST"/>
    <xsl:if test="$FIRST='true'">\begin{longtable}[l]{!{\color{blue}\vline}@{\hspace{0.5pt}}l}</xsl:if>
    <xsl:if test="$FIRST!='true'">\begin{tabular}{!{\color{blue}\vline}@{\hspace{0.5pt}}l}</xsl:if>
      <xsl:apply-templates mode="OCCURANCE" select=".">
        <xsl:with-param name="ELEMENTNAME" select="'li'"/>
        <xsl:with-param name="COLOR" select="'blue'"/>
      </xsl:apply-templates>
      <xsl:apply-templates mode="SIMPLECONTENT" select="xs:element|xs:sequence|xs:choice">
        <xsl:with-param name="CLASSNAME" select="$CLASSNAME"/>
      </xsl:apply-templates>
    <xsl:if test="$FIRST='true'">\end{longtable}</xsl:if>
    <xsl:if test="$FIRST!='true'">\end{tabular}\\</xsl:if>
  </xsl:template>

  <!-- element -->
  <xsl:template mode="SIMPLECONTENT" match="xs:element">
    <xsl:param name="FUNCTIONNAME" select="@name"/>
    <xsl:param name="CLASSNAME"/>
    <!-- name by not(ref) -->
    <xsl:if test="not(@ref)">\textbf{\texttt{$&lt;$<xsl:call-template name="CONUNDERSCORE"><xsl:with-param name="V" select="@name"/></xsl:call-template>$&gt;$}}</xsl:if>
    <!-- name by ref -->
    <xsl:if test="@ref">\textbf{\texttt{$&lt;$<xsl:call-template name="CONUNDERSCORE"><xsl:with-param name="V" select="@ref"/></xsl:call-template>$&gt;$}}
      (\ref{<xsl:value-of select="@ref"/>}, p. \pageref{<xsl:value-of select="@ref"/>})</xsl:if>
    <!-- occurence -->
    <xsl:apply-templates mode="OCCURANCE" select="."><xsl:with-param name="ELEMENTNAME" select="'span'"/></xsl:apply-templates>
    <!-- type -->
    <xsl:if test="@type">(Type: \texttt{<xsl:value-of select="@type"/>})</xsl:if>\\
    <!-- element attributes -->
    <xsl:if test="@name and not(@type)">
      <xsl:if test="xs:complexType/xs:attribute">
        <xsl:apply-templates mode="ELEMENTATTRIBUTE" select="xs:complexType/xs:attribute"/>
      </xsl:if>
    </xsl:if>
    <!-- documentation -->
    <xsl:apply-templates mode="ELEMENTANNOTATION" select="xs:annotation/xs:documentation"/>
    <!-- children -->
    <xsl:if test="@name and not(@type)">
      <xsl:apply-templates mode="SIMPLECONTENT" select="xs:complexType"/>
    </xsl:if>
  </xsl:template>

  <!-- element attributes -->
  <xsl:template mode="ELEMENTATTRIBUTE" match="xs:attribute">
    \hspace{2ex}\textbf{\texttt{<xsl:value-of select="@name"/>}}
    <xsl:if test="@use='required'"> \textit{[required]}</xsl:if>
    <xsl:if test="@use!='required'"> \textit{[optional]}</xsl:if> (Type: \texttt{<xsl:value-of select="@type"/>})\\
  </xsl:template>

  <!-- documentation -->
  <xsl:template mode="CLASSANNOTATION" match="xs:annotation/xs:documentation">
    <xsl:if test="@source!='doxygen' or not(@source) or $INCLUDEDOXYGEN='true'">
      <xsl:if test="@source='doxygen'"><xsl:text>

        \textbf{The following part is the C++ API docucmentation from Doxygen}

      </xsl:text></xsl:if>
      <xsl:apply-templates mode="CLONEDOC"/>
    </xsl:if>
  </xsl:template>

  <!-- documentation -->
  <xsl:template mode="ELEMENTANNOTATION" match="xs:annotation/xs:documentation">
    <xsl:if test="@source!='doxygen' or not(@source) or $INCLUDEDOXYGEN='true'">
      \hspace{3ex}
      \begin{minipage}[b]{\linewidth}
        <xsl:if test="@source='doxygen'"><xsl:text>

          \textbf{The following part is the C++ API docucmentation from Doxygen}

        </xsl:text></xsl:if>
        <xsl:apply-templates mode="CLONEDOC"/>
      \end{minipage}\\[1ex]
    </xsl:if>
  </xsl:template>



  <!-- clone doxygen xml/html part -->
  <xsl:template mode="CLONEDOC" match="*">
    <xsl:copy>
      <xsl:for-each select="@*">
        <xsl:copy/>
      </xsl:for-each>
      <xsl:apply-templates mode="CLONEDOC"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template mode="CLONEDOC" match="text()">
    <xsl:copy/>
  </xsl:template>

  <xsl:template mode="CLONEDOC" match="html:div[@class='para']"><xsl:text>

    </xsl:text><xsl:apply-templates mode="CLONEDOC"/><xsl:text>

  </xsl:text></xsl:template>

  <xsl:template mode="CLONEDOC" match="html:dl">
    \begin{description}
      <xsl:apply-templates mode="CLONEDOC"/>
    \end{description}
  </xsl:template>

  <xsl:template mode="CLONEDOC" match="html:dt">
    \item[<xsl:value-of select="."/>]
  </xsl:template>

  <xsl:template mode="CLONEDOC" match="html:dd">
    <xsl:apply-templates mode="CLONEDOC"/>
  </xsl:template>

  <xsl:template mode="CLONEDOC" match="html:img[@class='eqn']|html:img[@class='inlineeqn']">
    <xsl:value-of select="@alt"/>
  </xsl:template>

  <xsl:template mode="CLONEDOC" match="html:div[@class='htmlfigure']"/>

  <xsl:template mode="CLONEDOC" match="html:object[@class='latexfigure']">
    \begin{center}\includegraphics[width=<xsl:value-of select="@standby"/>]{<xsl:value-of select="@data"/>}\\<xsl:value-of select="@title"/>\end{center}
  </xsl:template>
 
</xsl:stylesheet>