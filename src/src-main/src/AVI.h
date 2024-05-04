#pragma once

DECLARE_HANDLE(HAVI);

namespace AVI
{
extern HAVI handle;

void	Init		(void);
void	Start		(void);
void	AddVideo	(void);
void	AddAudio	(void);
void	End		(void);
} // namespace AVI
