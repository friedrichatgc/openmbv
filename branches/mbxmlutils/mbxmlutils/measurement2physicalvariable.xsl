<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:mm="http://openmbv.berlios.de/MBXMLUtils/measurement"
  xmlns:xs="http://www.w3.org/2001/XMLSchema">

  <xsl:output method="xml" version="1.0" indent="yes"/>

  <xsl:template match="text()"/>

  <xsl:template match="/mm:measurement">
  
    <xsl:comment>
      <!-- Add this text to the output document to prevent others editing the output document manual -->
      This file is generated automatically using XSLT from measurement.xml.
      DO NOT EDIT!!!
    </xsl:comment>

    <xs:schema targetNamespace="http://openmbv.berlios.de/MBXMLUtils/physicalvariable"
      elementFormDefault="qualified"
      attributeFormDefault="unqualified"
      xmlns="http://openmbv.berlios.de/MBXMLUtils/physicalvariable"
      xmlns:xml="http://www.w3.org/XML/1998/namespace"
      xmlns:xs="http://www.w3.org/2001/XMLSchema">

      <!-- for xml:base attribute added by XInclude aware parser: include xml namespaces defining attribute xml:base -->
      <xs:import namespace="http://www.w3.org/XML/1998/namespace" schemaLocation="../http___www_w3_org/xml.xsd"/>

      <xs:include schemaLocation="parameter.xsd"/>
      <xs:include schemaLocation="embed.xsd"/>

      <!-- base type for a XML text element which is fully converted by octave.-->
      <xs:simpleType name="fullOctEval">
        <xs:restriction base="xs:token"/>
      </xs:simpleType>

      <!-- full octave evaluation which must evaluate to an floating number.-->
      <xs:simpleType name="floatFullOctEval">
        <xs:restriction base="fullOctEval"/>
      </xs:simpleType>

      <!-- full octave evaluation which must evaluate to an string.-->
      <xs:simpleType name="stringFullOctEval">
        <xs:restriction base="fullOctEval"/>
      </xs:simpleType>

      <!-- full octave evaluation which must evaluate to an integer.-->
      <xs:simpleType name="integerFullOctEval">
        <xs:restriction base="fullOctEval"/>
      </xs:simpleType>

      <!-- full octave evaluation which must evaluate to an boolean.-->
      <xs:simpleType name="booleanFullOctEval">
        <xs:restriction base="fullOctEval"/>
      </xs:simpleType>


      <!-- base type for a attribute which is partially converted by octave.
           Only the content between { and } ist converted by octave
           Inside { ... } the character { and } must be quoted qith \ -->
      <xs:simpleType name="partialOctEval">
        <xs:restriction base="xs:token"/>
      </xs:simpleType>

      <!-- partial octave evaluation which must evaluate to an floating number -->
      <xs:simpleType name="floatPartialOctEval">
        <xs:restriction base="partialOctEval"/>
      </xs:simpleType>

      <!-- partial octave evaluation which must evaluate to an string -->
      <xs:simpleType name="stringPartialOctEval">
        <xs:restriction base="partialOctEval"/>
      </xs:simpleType>

      <!-- partial octave evaluation which must evaluate to an integer -->
      <xs:simpleType name="integerPartialOctEval">
        <xs:restriction base="partialOctEval"/>
      </xs:simpleType>

      <!-- partial octave evaluation which must evaluate to an boolean -->
      <xs:simpleType name="booleanPartialOctEval">
        <xs:restriction base="partialOctEval"/>
      </xs:simpleType>

      <!-- partial octave evaluation which must evaluate to an string representing a valid variable name -->
      <xs:simpleType name="varnamePartialOctEval">
        <xs:restriction base="stringPartialOctEval"/>
      </xs:simpleType>

      <!-- partial octave evaluation which must evaluate to an string which represents a filename -->
      <xs:simpleType name="filenamePartialOctEval">
        <xs:restriction base="stringPartialOctEval"/>
      </xs:simpleType>

      <!-- just a special type to be able to detect such attributes by a schema-aware processor -->
      <xs:simpleType name="symbolicFunctionArgNameType">
        <xs:restriction base="varnamePartialOctEval"/>
      </xs:simpleType>

      <!-- the attribute type for vector argument dimension -->
      <xs:simpleType name="symbolicFunctionArgDimType">
        <xs:restriction base="integerFullOctEval"/>
      </xs:simpleType>




      <xs:simpleType name="partialOctaveString"><!--MFMF change in openmbv.xsd after the branch is merged to trunk-->
        <xs:restriction base="xs:token"/>
      </xs:simpleType>
      <xs:simpleType name="name"><!--MFMF change in openmbv.xsd after the branch is merged to trunk-->
        <xs:restriction base="xs:token"/>
      </xs:simpleType>
      <xs:simpleType name="string"><!--MFMF change in openmbv.xsd after the branch is merged to trunk-->
        <xs:restriction base="xs:token"/>
      </xs:simpleType>
      <!--MFMF more fixed (attribute types to octFullEval, ...) are needed on openmbv after merge to trunk-->





      <!-- add unit types -->
      <xsl:apply-templates mode="UNIT" select="mm:measure"/>

      <xs:group name="fromFileGroup">
        <xs:sequence>
          <xs:element name="fromFile" minOccurs="0"> <!-- minOcuurs is a workaround for a bug in libxml2-2.7 -->
            <xs:annotation><xs:documentation>
              Load the file referenced by 'href' and return it as a vector or matrix.
              All file formats of the octave 'load' command are supported and auto detected.
            </xs:documentation></xs:annotation>
            <xs:complexType>
              <xs:attribute name="href" use="required" type="filenamePartialOctEval"/>
            </xs:complexType>
          </xs:element>
        </xs:sequence>
      </xs:group>

      <xs:complexType mixed="true" name="matrix">
        <xs:annotation>
          <xs:documentation>
            Definition of a matrix of double values
          </xs:documentation>
          <xs:appinfo>
            <pattern xmlns="http://www.ascc.net/xml/schematron" name="test">
              <rule context="event">
                <assert test="not(node()[2])">
                  Only one child node is allowed: text or element.
                  <!-- regex for matrix expression -->
                </assert>
              </rule>
            </pattern>
          </xs:appinfo>
        </xs:annotation>
        <xs:choice minOccurs="0">
          <xs:group ref="xmlMatrixGroup"/>
          <xs:group ref="fromFileGroup"/>
        </xs:choice>
      </xs:complexType>

      <xs:group name="xmlMatrixGroup">
        <xs:choice>
          <xs:element name="xmlMatrix">
            <xs:complexType>
              <xs:sequence>
                <xs:element name="row" minOccurs="0" maxOccurs="unbounded">
                  <xs:complexType>
                    <xs:sequence>
                      <xs:element name="ele" minOccurs="0" maxOccurs="unbounded" type="floatFullOctEval"/>
                    </xs:sequence>
                  </xs:complexType>
                </xs:element>
              </xs:sequence>
              <xs:attribute ref="xml:base"/> <!-- allow a XInclude here -->
            </xs:complexType>
          </xs:element>
        </xs:choice>
      </xs:group>

      <!-- add matrix units -->
      <xsl:apply-templates mode="MATRIX" select="/mm:measurement/mm:measure"/>

      <xs:complexType mixed="true" name="vector">
        <xs:annotation>
          <xs:documentation>
            Definition of a vector of double values
          </xs:documentation>
          <xs:appinfo>
            <pattern xmlns="http://www.ascc.net/xml/schematron" name="test">
              <rule context="event">
                <assert test="not(node()[2])">
                  Only one child node is allowed: text or element.
                  <!-- regex for vector expression -->
                </assert>
              </rule>
            </pattern>
          </xs:appinfo>
        </xs:annotation>
        <xs:choice minOccurs="0">
          <xs:group ref="xmlVectorGroup"/>
          <xs:group ref="fromFileGroup"/>
        </xs:choice>
      </xs:complexType>

      <xs:group name="xmlVectorGroup">
        <xs:choice>
          <xs:element name="xmlVector">
            <xs:complexType>
              <xs:sequence>
                <xs:element name="ele" minOccurs="0" maxOccurs="unbounded" type="floatFullOctEval"/>
              </xs:sequence>
              <xs:attribute ref="xml:base"/> <!-- allow a XInclude here -->
            </xs:complexType>
          </xs:element>
        </xs:choice>
      </xs:group>

      <!-- add vector units -->
      <xsl:apply-templates mode="VECTOR" select="/mm:measurement/mm:measure"/>

      <!-- add scalar type -->
      <xs:complexType mixed="true" name="scalar">
        <xs:annotation>
          <xs:documentation>
            Definition of a scalar of double values
          </xs:documentation>
          <xs:appinfo>
            <pattern xmlns="http://www.ascc.net/xml/schematron" name="test">
              <rule context="event">
                <assert test="not(node()[2])">
                  Only one child node is allowed: text or element.
                  <!-- regex for scalar expression -->
                </assert>
              </rule>
            </pattern>
          </xs:appinfo>
        </xs:annotation>
        <xs:choice minOccurs="0">
          <xs:group ref="xmlScalarGroup"/>
        </xs:choice>
      </xs:complexType>

      <xs:group name="xmlScalarGroup">
        <!-- dummy group. just te be consistent with vector and matrix types -->
        <xs:sequence/> <!-- some parsers have problems with no child element in xs:group -->
      </xs:group>

      <!-- add scalar units -->
      <xsl:apply-templates mode="SCALAR" select="/mm:measurement/mm:measure"/>

      <!-- unknown scalar/vector/matrix -->
      <xs:complexType name="unknownScalar" mixed="true">
        <xs:annotation>
          <xs:documentation>
            A scalar value in a unknown unit. The value is evaluated by the
            octave string given in the convertUnit attribute.
          </xs:documentation>
        </xs:annotation>
        <xs:complexContent>
          <xs:extension base="scalar">
            <xs:attribute name="convertUnit" type="xs:string"/> <!-- convertUnit is handeled specially in the preprocessor -->
          </xs:extension>
        </xs:complexContent>
      </xs:complexType>

      <xs:complexType name="unknownVector" mixed="true">
        <xs:annotation>
          <xs:documentation>
            A vector value in a unknown unit. The value is evaluated by the
            octave string given in the convertUnit attribute.
          </xs:documentation>
        </xs:annotation>
        <xs:complexContent>
          <xs:extension base="vector">
            <xs:attribute name="convertUnit" type="xs:string"/> <!-- convertUnit is handeled specially in the preprocessor -->
          </xs:extension>
        </xs:complexContent>
      </xs:complexType>

      <xs:complexType name="unknownMatrix" mixed="true">
        <xs:annotation>
          <xs:documentation>
            A matrix value in a unknown unit. The value is evaluated by the
            octave string given in the convertUnit attribute.
          </xs:documentation>
        </xs:annotation>
        <xs:complexContent>
          <xs:extension base="matrix">
            <xs:attribute name="convertUnit" type="xs:string"/> <!-- convertUnit is handeled specially in the preprocessor -->
          </xs:extension>
        </xs:complexContent>
      </xs:complexType>

      <!-- rotation matrix -->
      <xs:complexType mixed="true" name="rotationMatrix">
        <xs:annotation><xs:documentation>
          A 3x3 rotation matrix.
        </xs:documentation></xs:annotation>
        <xs:complexContent>
          <xs:extension base="matrix">
            <!-- support everything supported by a normal matrix e.g.
              <rot>[1,0,0;0,1,0;0,0,1]</rot>
              <rot><xmlMatrix>...</xmlMatrix></rot>
              and the following ...
            -->
            <xs:choice minOccurs="0">
              <xs:element name="aboutX" type="angleScalar"/>
              <xs:element name="aboutY" type="angleScalar"/>
              <xs:element name="aboutZ" type="angleScalar"/>
              <xs:element name="cardan">
                <xs:complexType>
                  <xs:sequence>
                    <xs:element name="alpha" type="floatFullOctEval"/>
                    <xs:element name="beta" type="floatFullOctEval"/>
                    <xs:element name="gamma" type="floatFullOctEval"/>
                  </xs:sequence>
                  <xs:attributeGroup ref="angleMeasure"/>
                </xs:complexType>
              </xs:element>
              <xs:element name="euler">
                <xs:complexType>
                  <xs:sequence>
                    <xs:element name="PHI" type="floatFullOctEval"/>
                    <xs:element name="theta" type="floatFullOctEval"/>
                    <xs:element name="phi" type="floatFullOctEval"/>
                  </xs:sequence>
                  <xs:attributeGroup ref="angleMeasure"/>
                </xs:complexType>
              </xs:element>
            </xs:choice>
          </xs:extension>
        </xs:complexContent>
      </xs:complexType>

      <!-- the MBXMLUtils XML representation of a casadi SXFunction.
           This may be replaced later by MathML or OpenMath if casadi supports it -->
      <xs:group name="symbolicFunctionXMLElement">
        <xs:annotation>
          <xs:documentation>
            A symbolic function definition which is evaluated at runtime using dynamic input parameters
            provided by the runtime. The representation of the symbolic function must either be given by
            the MBXMLUtils notation for CasADi::SXFunction or by an octave expression (using the SWIG
            octave interface of CasADi). Using a octave expression you have full access to other scalar,
            vector or matrix parameters. For each input parameter an attribute named 'xyz' of type 'symbolicFunctionArgNameType'
            must be defined which set the name of this input parameter for the access in the octave expression.
            For each vector input paramter moreover an attribure named 'xyzDim' must be defined
            which defines the vector dimension of this input.
            (The XML schema must also define a fixed (hidden) attribute named 'xyzNr' which is set the integer
            number corresponding to the argument number of the function)
          </xs:documentation>
        </xs:annotation>
        <xs:choice>
          <xs:any minOccurs="0" namespace="http://openmbv.berlios.de/MBXMLUtils/CasADi" processContents="lax"/>
          <xs:group ref="xmlScalarGroup"/>
          <xs:group ref="xmlVectorGroup"/>
          <xs:group ref="xmlMatrixGroup"/>
        </xs:choice>
      </xs:group>

    </xs:schema>

  </xsl:template>

  <!-- add unit types -->
  <xsl:template mode="UNIT" match="/mm:measurement/mm:measure">
    <xs:attributeGroup>
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>Measure</xsl:attribute>
      <xs:annotation>
        <xs:documentation>
          Enumeration of known units of <xsl:value-of select="@name"/>
        </xs:documentation>
      </xs:annotation>
      <xs:attribute name="unit">
        <xsl:attribute name="default">
          <xsl:value-of select="@SIunit"/>
        </xsl:attribute>
        <xs:simpleType>
          <xs:restriction base="xs:token">
            <xsl:apply-templates mode="UNIT" select="mm:unit"/>
          </xs:restriction>
        </xs:simpleType>
      </xs:attribute>
    </xs:attributeGroup>
  </xsl:template>

  <!-- add unit types -->
  <xsl:template mode="UNIT" match="/mm:measurement/mm:measure/mm:unit">
    <xs:enumeration>
      <xsl:attribute name="value">
        <xsl:value-of select="@name"/>
      </xsl:attribute>
    </xs:enumeration>
  </xsl:template>

  <!-- add matrix units -->
  <xsl:template mode="MATRIX" match="/mm:measurement/mm:measure">
    <xs:complexType mixed="true">
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>Matrix</xsl:attribute>
      <xs:annotation>
        <xs:documentation>
          Matrix measure of <xsl:value-of select="@name"/>
        </xs:documentation>
      </xs:annotation>
      <xs:complexContent>
        <xs:extension base="matrix">
          <xs:attributeGroup>
            <xsl:attribute name="ref">
              <xsl:value-of select="@name"/>Measure</xsl:attribute>
          </xs:attributeGroup>
        </xs:extension>
      </xs:complexContent>
    </xs:complexType>
  </xsl:template>

  <!-- add vector units -->
  <xsl:template mode="VECTOR" match="/mm:measurement/mm:measure">
    <xs:complexType mixed="true">
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>Vector</xsl:attribute>
      <xs:annotation>
        <xs:documentation>
          Vector measure of <xsl:value-of select="@name"/>
        </xs:documentation>
      </xs:annotation>
      <xs:complexContent>
        <xs:extension base="vector">
          <xs:attributeGroup>
            <xsl:attribute name="ref">
              <xsl:value-of select="@name"/>Measure</xsl:attribute>
          </xs:attributeGroup>
        </xs:extension>
      </xs:complexContent>
    </xs:complexType>
  </xsl:template>

  <!-- add scalar units -->
  <xsl:template mode="SCALAR" match="/mm:measurement/mm:measure">
    <xs:complexType mixed="true">
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>Scalar</xsl:attribute>
      <xs:annotation>
        <xs:documentation>
          Scalar measure of <xsl:value-of select="@name"/>
        </xs:documentation>
      </xs:annotation>
      <xs:complexContent>
        <xs:extension base="scalar">
          <xs:attributeGroup>
            <xsl:attribute name="ref">
              <xsl:value-of select="@name"/>Measure</xsl:attribute>
          </xs:attributeGroup>
        </xs:extension>
      </xs:complexContent>
    </xs:complexType>
  </xsl:template>

</xsl:stylesheet>