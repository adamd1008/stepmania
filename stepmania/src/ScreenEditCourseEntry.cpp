#include "global.h"
#include "ScreenEditCourseEntry.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
#include "CommonMetrics.h"
#include "GameManager.h"
#include "song.h"

enum EditCourseEntryRow
{
	ROW_SONG_GROUP, 
	ROW_SONG, 
	ROW_BASE_DIFFICULTY, 
	ROW_LOW_METER,
	ROW_HIGH_METER, 
	ROW_BEST_WORST_VALUE, 
	ROW_SET_MODS, 
	ROW_DONE,
	NUM_EditCourseEntryRow
};
#define FOREACH_EditCourseEntryRow( i ) FOREACH_ENUM( EditCourseEntryRow, NUM_EditCourseEntryRow, i )

REGISTER_SCREEN_CLASS( ScreenEditCourseEntry );
ScreenEditCourseEntry::ScreenEditCourseEntry( CString sName ) : ScreenOptions( sName )
{
	LOG->Trace( "ScreenEditCourseEntry::ScreenEditCourseEntry()" );
}

void ScreenEditCourseEntry::Init()
{
	ScreenOptions::Init();

	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	
	def.name = "Song Group";
	def.choices.clear();
	vector<CString> vsSongGroups;
	SONGMAN->GetSongGroupNames( vsSongGroups );
	FOREACH_CONST( CString, vsSongGroups, s )
		def.choices.push_back( *s );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Song";
	def.choices.clear();
	def.choices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Base Difficulty";
	def.choices.clear();
	FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), dc )
		def.choices.push_back( DifficultyToThemedString(*dc) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Low Meter";
	def.choices.clear();
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.choices.push_back( ssprintf("%i",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "High Meter";
	def.choices.clear();
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.choices.push_back( ssprintf("%i",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Best/Worst Value";
	def.choices.clear();
	for( int i=1; i<=20; i++ )
		def.choices.push_back( ssprintf("%i",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Set Mods";
	def.choices.clear();
	def.choices.push_back( "Set Mods" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	ScreenOptions::InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );

	AfterChangeValueInRow( GAMESTATE->m_MasterPlayerNumber );
}

void ScreenEditCourseEntry::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenEditCourseEntry::AfterChangeValueInRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( pn );
	Course *pCourse = GAMESTATE->m_pCurCourse;
	CourseEntry &ce = pCourse->m_entries[ GAMESTATE->m_iEditCourseEntryIndex ];
	CString sSongGroup;

	switch( m_iCurrentRow[pn] )
	{
	case ROW_SONG_GROUP:
		// export song group
		{
			OptionRow &row = *m_pRows[ROW_SONG_GROUP];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			sSongGroup = row.GetRowDef().choices[ iChoice ];
		}
		// refresh songs
		{
			OptionRow &row = *m_pRows[ROW_SONG];
			OptionRowDefinition def = row.GetRowDef();
			def.choices.clear();
			vector<Song*> vpSongs;
			SONGMAN->GetSongs( vpSongs, sSongGroup );
			FOREACH_CONST( Song*, vpSongs, s )
				def.choices.push_back( (*s)->GetTranslitFullTitle() );
			vector<Song*>::const_iterator iter = find( vpSongs.begin(), vpSongs.end(), ce.pSong );
			if( iter != vpSongs.end() )
			{
				int iSongIndex = iter - vpSongs.begin();
				row.SetOneSharedSelection( iSongIndex );
			}
			else
			{
				row.SetOneSharedSelection( 0 );
			}
			row.Reload( def );
		}
		// fall through
	case ROW_SONG:
		// export song
		{
			OptionRow &row = *m_pRows[ROW_SONG];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			ce.pSong = SONGMAN->GetAllSongs()[iChoice];
		}
		// fall through
	case ROW_BASE_DIFFICULTY:
		// export difficulty
		{
			OptionRow &row = *m_pRows[ROW_BASE_DIFFICULTY];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			ce.baseDifficulty = DIFFICULTIES_TO_SHOW.GetValue()[iChoice];
		}
		// fall through
	case ROW_LOW_METER:
		// export low meter
		{
		}
		// fall through
	case ROW_HIGH_METER:
		// export high meter
		{
		}
		// fall through
	case ROW_BEST_WORST_VALUE:
		// export best/worst value
		{
		}
		// fall through
	case ROW_SET_MODS:
		// fall through
	default:
		; // nothing left to do
	}
}

void ScreenEditCourseEntry::ImportOptions( int row, const vector<PlayerNumber> &vpns )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
	CourseEntry &ce = pCourse->m_entries[ GAMESTATE->m_iEditCourseEntryIndex ];

	// import entry song
	{
		int iSongIndex = -1;
		vector<Song*>::const_iterator iter = find( SONGMAN->GetAllSongs().begin(), SONGMAN->GetAllSongs().end(), ce.pSong );
		if( iter != SONGMAN->GetAllSongs().end() )
			iSongIndex = iter - SONGMAN->GetAllSongs().begin();
		OptionRow &row = *m_pRows[ROW_SONG];
		if( iSongIndex != -1 )
			row.SetOneSharedSelection( iSongIndex );
	}
	/*
	// import entry difficulty
	{
		int iDifficultyIndex = -1;
		vector<Difficulty>::const_iterator iter = find( DIFFICULTIES_TO_SHOW.GetValue().begin(), DIFFICULTIES_TO_SHOW.GetValue().end(), ce.difficulty );
		if( iter != DIFFICULTIES_TO_SHOW.GetValue().end() )
			iDifficultyIndex = iter - DIFFICULTIES_TO_SHOW.GetValue().begin();
		OptionRow &row = *m_pRows[ROW_DIFFICULTY];
		if( iDifficultyIndex != -1 )
			row.SetOneSharedSelection( iDifficultyIndex );
	}
	*/
}

void ScreenEditCourseEntry::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenEditCourseEntry::GoToNextScreen()
{
	switch( m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] )
	{
	case ROW_SONG_GROUP: 
	case ROW_SONG: 
	case ROW_BASE_DIFFICULTY: 
	case ROW_LOW_METER:
	case ROW_HIGH_METER: 
	case ROW_BEST_WORST_VALUE: 
		break;
	case ROW_SET_MODS:
		SCREENMAN->SetNewScreen( "ScreenEditCourseMods" );
		break;
	case ROW_DONE:
		SCREENMAN->SetNewScreen( "ScreenEditCourse" );
		break;
	}
}

void ScreenEditCourseEntry::GoToPrevScreen()
{
	SCREENMAN->SetNewScreen( "ScreenEditCourse" );
}

void ScreenEditCourseEntry::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	switch( m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] )
	{
	case ROW_SONG_GROUP: 
	case ROW_SONG: 
	case ROW_BASE_DIFFICULTY: 
	case ROW_LOW_METER:
	case ROW_HIGH_METER: 
	case ROW_BEST_WORST_VALUE: 
		break;
	case ROW_SET_MODS:
	case ROW_DONE:
		ScreenOptions::ProcessMenuStart( pn, type );
		break;
	}
}

/*
 * (c) 2002-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
