


Type cfgVal
  devName As String
  fPath As String
  fName As String
  fExt As String
  fileName As String
  IndeX As Integer  'IndeX point
  hdrY As Integer
  hdrX As Integer  'header point
  offY As Integer
  offX(64) As Integer 'data point offsetX 0~63 from hdrX, value is print or not
  endX As Integer  'data end check point offset from hdrX
  endX_blankSkip As Integer
  endText As Integer 'endText value is print or not
  elsText As Integer 'elsText value is print or not
End Type

Dim i As Integer
Dim j As Integer

Dim DQM As String       'char["], double quotation marks
Dim SEP As String       'separator


Function initArray(ByRef aVal() As Integer, size As Integer)
    For i = 0 To size - 1
        aVal(i) = 0
    Next
End Function

Function initCfgV(lcfgV As cfgVal)
    Call initArray(lcfgV.offX, 64)
End Function

Function genFile(lcfgV As cfgVal)
    Dim loffY As Integer
    Dim lendX As Integer
    Dim prnCnt As Integer
    
    loffY = lcfgV.offY + lcfgV.hdrY
    lendX = lcfgV.endX + lcfgV.hdrX
    prnCnt = 0

    lcfgV.fileName = lcfgV.fPath & lcfgV.fName & ActiveSheet.Cells(lcfgV.hdrY, lcfgV.hdrX) & lcfgV.fExt
    Open lcfgV.fileName For Output As #1
    Print #1, "##" & lcfgV.devName & ActiveSheet.Cells(lcfgV.hdrY, lcfgV.hdrX) & "##"
    For i = loffY To 3000
        If (ActiveSheet.Cells(i, lendX) = "end") Then
          If (lcfgV.endText = 1) Then
            Print #1, prnCnt & SEP & ActiveSheet.Cells(lcfgV.hdrY + 1, lcfgV.hdrX) & SEP
          End If
          Exit For
        End If

        If ((lcfgV.endX_blankSkip = 0) Or (ActiveSheet.Cells(i, lendX) <> "")) Then
          Print #1, prnCnt & SEP;
          prnCnt = prnCnt + 1
          For j = 0 To 63
            If (lcfgV.offX(j) = 1) Then
              If (ActiveSheet.Cells(i, lcfgV.hdrX + j) <> "") Then
                  If (LEFT(ActiveSheet.Cells(i, lcfgV.hdrX + j),2) = "0x") Then
                    Print #1, HEX2DEC(RIGHT(ActiveSheet.Cells(i, lcfgV.hdrX + j),8)) & SEP;
                  Else
                    Print #1, ActiveSheet.Cells(i, lcfgV.hdrX + j) & SEP;
                  End If
              Else
                If (lcfgV.elsText = 1) Then
                  Print #1, ActiveSheet.Cells(lcfgV.hdrY + 2, lcfgV.hdrX) & SEP;
                End If
              End If
            End If
          Next j
          Print #1,
        End If
    Next i
    Close #1
End Function

Sub fileWrite()
    
    Application.ScreenUpdating = False  '화면 업데이트 (일시) 중지

    DQM = Chr(34)
    SEP = ","
    
    Dim wOrg As Worksheet
    Set wOrg = Worksheets("org")
    Dim cfgV As cfgVal

    cfgV.fPath = ThisWorkbook.Path & Application.PathSeparator & ActiveSheet.Cells(3, 1) & ".initValue" & Application.PathSeparator
    cfgV.fName = ActiveSheet.Cells(5, 2)
    cfgV.fExt = ".init"
    cfgV.devName = ActiveSheet.Cells(6, 2)
    'MsgBox strPath


    If ((ActiveSheet.Cells(5, 1) = "EVG") Or (ActiveSheet.Cells(5, 1) = "EVS")) Then

        cfgV.hdrY = 7
        cfgV.offY = 3
        cfgV.IndeX = 3

        'evg0 config
        Call initCfgV(cfgV)
        cfgV.hdrX = 4
        cfgV.offX(0) = 1
        cfgV.offX(3) = 1
        cfgV.endX = 0
        cfgV.endX_blankSkip = 1
        cfgV.endText = 0
        cfgV.elsText = 0
        Call genFile(cfgV)

        'evg0 seqA
        Call initCfgV(cfgV)
        cfgV.hdrX = 9
        cfgV.offX(0) = 1
        cfgV.endX = 0
        cfgV.endX_blankSkip = 0
        cfgV.endText = 1
        cfgV.elsText = 1
        Call genFile(cfgV)
        cfgV.hdrX = 10
        cfgV.endX = -1
        Call genFile(cfgV)

        'evg0 seqB
        Call initCfgV(cfgV)
        cfgV.hdrX = 12
        cfgV.offX(0) = 1
        cfgV.endX = 0
        cfgV.endX_blankSkip = 0
        cfgV.endText = 1
        cfgV.elsText = 1
        Call genFile(cfgV)
        cfgV.hdrX = 13
        cfgV.endX = -1
        Call genFile(cfgV)

        'evg0 config
        Call initCfgV(cfgV)
        cfgV.hdrX = 15
        cfgV.offX(0) = 1
        cfgV.offX(1) = 1
        cfgV.endX = 0
        cfgV.endX_blankSkip = 1
        cfgV.endText = 0
        cfgV.elsText = 0
        Call genFile(cfgV)

        'evr0 mapping RAM
        Call initCfgV(cfgV)
        cfgV.hdrX = 18
        cfgV.offX(0) = 1
        cfgV.endX = 0
        cfgV.endX_blankSkip = 0
        cfgV.endText = 0
        cfgV.elsText = 0
        Call genFile(cfgV)
        cfgV.hdrX = 19
        cfgV.endX = 0
        Call genFile(cfgV)

    ElseIf ((ActiveSheet.Cells(5, 1) = "EVR") Or (ActiveSheet.Cells(5, 1) = "ZQ900")) Then

        Call initCfgV(cfgV)
        cfgV.hdrX = 21
        cfgV.offX(0) = 1
        cfgV.endX = 0
        cfgV.endX_blankSkip = 0
        cfgV.endText = 0
        cfgV.elsText = 0
        Call genFile(cfgV)
        cfgV.hdrX = 22
        cfgV.endX = 0
        Call genFile(cfgV)

    End If
    

End Sub







